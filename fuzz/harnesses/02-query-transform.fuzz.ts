/**
 * Harness 02 — NoSQL Query-Transform Pipeline Fuzzer
 *
 * Target: CoursesService.getAllCourses — the query-building pipeline:
 *
 *   const queryObj = {...queryString};
 *   if (queryObj.search) delete queryObj.search;
 *   let queryStr = JSON.stringify(queryObj);
 *   queryStr = queryStr.replace(/\b(gte|gt|lte|lt)\b/g, m => `$${m}`);
 *   const result = JSON.parse(queryStr);                  // ← can throw
 *   coursesModel.find({...result, ...searchQuery})         // ← MongoDb filter
 *
 * What we're looking for:
 *   1. JSON.parse throwing SyntaxError due to the regex replace corrupting JSON
 *      (e.g. replacing operator names inside string values that look like JSON keys)
 *   2. MongoDB operator injection paths: inputs that produce $where, $regex,
 *      $gt, $ne etc. in the final filter without going through the allowlist.
 *   3. ReDoS via the search field ($regex without escaping).
 *   4. Prototype pollution via crafted field names (__proto__, constructor, etc.).
 *
 * This harness isolates the pure data-transformation logic (no DB call) so it
 * runs fast without any external dependencies.
 *
 * Safe outcomes (not bugs):
 *   - SyntaxError from JSON.parse — already NOSQLI-001; we log and continue.
 *   - Any TypeError from the spread — we log and continue.
 *
 * Findings:
 *   - Any other Error type.
 *   - Final filter containing keys that start with '$' beyond the four allowed
 *     operators (a logical assertion checked inline).
 */

import { FuzzedDataProvider } from '@jazzer.js/core';

// ---------------------------------------------------------------------------
// Exact replica of the vulnerable query-transform pipeline extracted from
// CoursesService.getAllCourses (SeNest/src/courses/courses.service.ts).
// We test the logic in isolation so the harness needs no DB or NestJS runtime.
// ---------------------------------------------------------------------------

interface QueryStringDto {
  price?: unknown;
  language?: unknown;
  level?: unknown;
  search?: unknown;
  [key: string]: unknown;   // extra fields that bypass DTO typing
}

function runQueryTransform(queryString: QueryStringDto): object | null {
  const queryObj: QueryStringDto = { ...queryString };
  if (queryObj.search) delete queryObj.search;

  let queryStr = JSON.stringify(queryObj);
  // The vulnerable replace: substitutes bare words gte/gt/lte/lt with $-prefixed versions.
  queryStr = queryStr.replace(/\b(gte|gt|lte|lt)\b/g, (match) => `$${match}`);

  // JSON.parse can throw if the replace corrupted the JSON.
  const parsed = JSON.parse(queryStr);

  // Build the search sub-query exactly as the service does.
  const searchQuery = {
    title: {
      $regex: (queryString.search as string) || '',
      $options: 'i',
    },
  };

  // The final MongoDB filter — what actually gets sent to the DB.
  return { ...parsed, ...searchQuery };
}

// ---------------------------------------------------------------------------
// Allowed MongoDB operator keys that the app intentionally produces.
// Any other $-prefixed key in the output represents an injection.
// ---------------------------------------------------------------------------
const ALLOWED_DOLLAR_KEYS = new Set(['$gt', '$gte', '$lt', '$lte', '$regex', '$options']);

function assertNoInjection(filter: object): void {
  function walk(obj: unknown, path: string): void {
    if (obj === null || typeof obj !== 'object') return;
    for (const [k, v] of Object.entries(obj as Record<string, unknown>)) {
      const fullPath = path ? `${path}.${k}` : k;
      if (k.startsWith('$') && !ALLOWED_DOLLAR_KEYS.has(k)) {
        // An operator that the transform pipeline should not have produced.
        throw new Error(
          `INJECTION DETECTED: unexpected MongoDB operator "${k}" at path "${fullPath}"`
        );
      }
      if (typeof v === 'object' && v !== null) walk(v, fullPath);
    }
  }
  walk(filter, '');
}

// ---------------------------------------------------------------------------
// Harness entry point
// ---------------------------------------------------------------------------

export function fuzz(data: Buffer): void {
  const provider = new FuzzedDataProvider(data);

  const strategy = provider.consumeIntegralInRange(0, 4);

  switch (strategy) {
    case 0: fuzzKnownFields(provider);     break;
    case 1: fuzzArbitraryFields(provider); break;
    case 2: fuzzNestedOperators(provider); break;
    case 3: fuzzDollarPrefixKeys(provider); break;
    case 4: fuzzSearchReDoS(provider);     break;
  }
}

// ---------------------------------------------------------------------------

function fuzzKnownFields(provider: FuzzedDataProvider): void {
  // Build a query object with the four documented DTO fields filled from fuzz data.
  const q: QueryStringDto = {};

  if (provider.consumeBoolean()) {
    // price as a plain number OR as a nested object (the bracket-notation attack)
    if (provider.consumeBoolean()) {
      q.price = provider.consumeIntegralInRange(-1e9, 1e9);
    } else {
      // Nested object simulating ?price[gt]=100 parsed by qs
      const nested: Record<string, unknown> = {};
      const op = ['gt', 'gte', 'lt', 'lte', '$gt', '$gte', '$lt', '$lte', '$ne', '$where', 'gt\0'][
        provider.consumeIntegralInRange(0, 10)
      ];
      nested[op] = provider.consumeIntegralInRange(0, 1e6);
      q.price = nested;
    }
  }
  if (provider.consumeBoolean()) q.language = provider.consumeString(64);
  if (provider.consumeBoolean()) q.level    = provider.consumeString(64);
  if (provider.consumeBoolean()) q.search   = provider.consumeString(256);

  runTransformSafely(q);
}

function fuzzArbitraryFields(provider: FuzzedDataProvider): void {
  // Simulate what happens when extra query params bypass DTO typing.
  const q: QueryStringDto = {};
  const fieldCount = provider.consumeIntegralInRange(0, 8);

  for (let i = 0; i < fieldCount; i++) {
    const key = provider.consumeString(provider.consumeIntegralInRange(1, 32));
    if (provider.consumeBoolean()) {
      q[key] = provider.consumeString(64);
    } else {
      // Nested object value
      const innerKey = provider.consumeString(16);
      const inner: Record<string, unknown> = {};
      inner[innerKey] = provider.consumeString(32);
      q[key] = inner;
    }
  }

  runTransformSafely(q);
}

function fuzzNestedOperators(provider: FuzzedDataProvider): void {
  // Directly construct the nested-operator structures that qs parses from
  // bracket-notation query strings like ?price[gt]=0&price[ne]=999
  const operators = [
    'gt', 'gte', 'lt', 'lte',
    '$gt', '$gte', '$lt', '$lte',
    '$ne', '$nin', '$exists', '$where',
    '$regex', '$options', '$elemMatch',
    '__proto__', 'constructor', 'prototype',
  ];

  const q: QueryStringDto = {};
  const opCount = provider.consumeIntegralInRange(1, 4);
  for (let i = 0; i < opCount; i++) {
    const field = ['price', 'language', 'level', 'title', '$where', '__proto__'][
      provider.consumeIntegralInRange(0, 5)
    ];
    const op = operators[provider.consumeIntegralInRange(0, operators.length - 1)];
    const value = provider.consumeBoolean()
      ? provider.consumeString(64)
      : provider.consumeIntegralInRange(0, 1e6);

    if (typeof q[field] === 'object' && q[field] !== null) {
      (q[field] as Record<string, unknown>)[op] = value;
    } else {
      q[field] = { [op]: value };
    }
  }

  runTransformSafely(q);
}

function fuzzDollarPrefixKeys(provider: FuzzedDataProvider): void {
  // Directly inject $-prefixed keys to see if they survive into the final filter.
  const dollarKeys = [
    '$where', '$expr', '$jsonSchema', '$text', '$comment',
    '$mod', '$type', '$size', '$all', '$elemMatch',
  ];

  const q: QueryStringDto = {};
  const key = dollarKeys[provider.consumeIntegralInRange(0, dollarKeys.length - 1)];
  q[key] = provider.consumeString(128);

  runTransformSafely(q);
}

function fuzzSearchReDoS(provider: FuzzedDataProvider): void {
  // Inject catastrophic backtracking patterns into the search field.
  // These are placed into MongoDB $regex unescaped — see REDOS-001.
  const catastrophicPatterns = [
    '^(a+)+$',
    '^([a-zA-Z]+)*$',
    '^(a|a?)+$',
    '(.*a){20,}',
    '^(.*,){1000}',
    // Mix random fuzz bytes with a known-catastrophic skeleton.
    provider.consumeString(32) + '(.*' + provider.consumeString(8) + '){10,}' + provider.consumeString(32),
  ];
  const pattern = catastrophicPatterns[provider.consumeIntegralInRange(0, catastrophicPatterns.length - 1)];
  runTransformSafely({ search: pattern });
}

// ---------------------------------------------------------------------------
// Safe runner: classifies errors.
// ---------------------------------------------------------------------------

function runTransformSafely(q: QueryStringDto): void {
  let filter: object | null = null;
  try {
    filter = runQueryTransform(q);
  } catch (err: unknown) {
    if (err instanceof SyntaxError || err instanceof TypeError) {
      // SyntaxError: JSON.parse failed after regex replaced content inside a string value.
      // TypeError: spread of non-object.
      // Both are known bugs (NOSQLI-001) — log but don't re-throw (the PoC covers them).
      return;
    }
    throw err; // Unexpected error type → Jazzer.js records as a finding.
  }

  if (filter !== null) {
    try {
      assertNoInjection(filter);
    } catch (injErr) {
      // Injection assertion — re-throw so Jazzer.js records the input.
      throw injErr;
    }
  }
}
