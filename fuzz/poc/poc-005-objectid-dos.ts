/**
 * PoC-005 — DOS-001: MongoDB CastError via Invalid ObjectId
 *
 * Root cause (courses.service.ts):
 *
 *   async getCourse(id: string): Promise<returnCourseDto> {
 *     const course = this.coursesModel.findById(id).populate("instructor");
 *     // Mongoose throws a CastError synchronously if `id` is not a valid ObjectId.
 *     // This propagates as an unhandled rejection → NestJS returns 500.
 *   }
 *
 * GET /courses/:id has no authentication requirement, so this is an
 * unauthenticated denial-of-service vector.
 *
 * The response difference:
 *   Valid ObjectId (not found): 404 NotFoundException (or null with current bug — see NPE-001)
 *   Invalid ObjectId:           500 Internal Server Error with CastError stack trace
 *
 * This PoC demonstrates the CastError using Mongoose directly.
 *
 * Run: npx ts-node fuzz/poc/poc-005-objectid-dos.ts
 */

import mongoose, { Schema, Model, Document } from 'mongoose';
import { MongoMemoryServer } from 'mongodb-memory-server';
import { isValidObjectId } from 'mongoose';

interface ICourse extends Document {
  title: string;
  instructor: mongoose.Types.ObjectId;
}

const CourseSchema = new Schema<ICourse>({
  title: String,
  instructor: { type: Schema.Types.ObjectId, ref: 'Users' },
});

function section(title: string): void {
  console.log('\n' + '─'.repeat(65));
  console.log(title);
  console.log('─'.repeat(65));
}

const INVALID_IDS = [
  'NOT_AN_OBJECT_ID',
  '../../etc/passwd',
  ''; eval(1)',
  '{$gt:""}',
  'undefined',
  'null',
  '000',          // too short
  'g'.repeat(24), // right length, wrong chars
  ' '.repeat(24), // whitespace
  '\x00'.repeat(24),
  '__proto__',
  'a'.repeat(1000),
  '',
];

const VALID_IDS = [
  '000000000000000000000001',
  '507f1f77bcf86cd799439011',
  'ffffffffffffffffffffffff',
];

(async () => {
  console.log('PoC-005 — MongoDB CastError DoS via Invalid ObjectId');
  console.log('SECURITY.md reference: DOS-001');
  console.log('Route: GET /courses/:id (no authentication required)');

  const mongod = await MongoMemoryServer.create();
  await mongoose.connect(mongod.getUri());
  const CourseModel = mongoose.model<ICourse>('Courses', CourseSchema);

  // --- Demonstrate CastError for each invalid ID ---
  section('Vulnerable path: findById() without isValidObjectId() guard');

  let castErrorCount = 0;
  for (const id of INVALID_IDS) {
    try {
      await CourseModel.findById(id);
      console.log(`  [NO ERROR] findById("${id.slice(0, 30)}…")`);
    } catch (err: unknown) {
      if (err instanceof Error && err.name === 'CastError') {
        castErrorCount++;
        console.log(`  ✓ CastError: findById("${id.slice(0, 30)}")`);
      } else {
        console.log(`  [OTHER ERROR] ${(err as Error)?.name}: ${(err as Error)?.message?.slice(0, 60)}`);
      }
    }
  }
  console.log(`\n  ${castErrorCount}/${INVALID_IDS.length} invalid IDs caused CastError (500 in production).`);

  // --- Fixed path: guard with isValidObjectId() ---
  section('Fixed path: isValidObjectId() guard before findById()');

  for (const id of INVALID_IDS) {
    if (!isValidObjectId(id)) {
      console.log(`  ✓ isValidObjectId("${id.slice(0, 30)}") = false → return 400, no DB call`);
    } else {
      console.log(`  [WARN] isValidObjectId("${id.slice(0, 30)}") = true — unexpected`);
    }
  }

  for (const id of VALID_IDS) {
    if (isValidObjectId(id)) {
      console.log(`  ✓ isValidObjectId("${id}") = true → proceed to findById`);
    } else {
      console.log(`  [WARN] isValidObjectId("${id}") = false — unexpected`);
    }
  }

  // --- Show response difference ---
  section('Response difference (reproduces information leakage)');
  console.log('Without the fix, an attacker can distinguish:');
  console.log('  400 / no response → valid ObjectId format (course not found = 404)');
  console.log('  500 CastError     → invalid ObjectId format');
  console.log('This leaks the ObjectId format requirement and produces noisy 500 logs.');

  // --- PoC HTTP requests ---
  section('HTTP requests to reproduce against a running server');
  console.log('# Trigger CastError (500) — no auth required:');
  INVALID_IDS.slice(0, 4).forEach(id => {
    if (id && id.length < 40) {
      console.log(`  curl http://localhost:3000/courses/${encodeURIComponent(id)}`);
    }
  });
  console.log('\n# Valid ObjectId format — returns 404 (or 500 via NPE-001):');
  console.log('  curl http://localhost:3000/courses/000000000000000000000001');

  section('SUMMARY');
  console.log('Fix: Add isValidObjectId(id) check in getCourse, updateCourse,');
  console.log('     deleteCourse, and insertImage before any findById() call.');
  console.log('See SECURITY.md §DOS-001 for the corrected code.');

  await mongoose.disconnect();
  await mongod.stop();
})();
