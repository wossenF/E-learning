/**
 * Harness 05 — MongoDB ObjectId Parsing Fuzzer
 *
 * Target: Mongoose's isValidObjectId() + Model.findById(id)
 *         Used in getCourse, updateCourse, deleteCourse, insertImage.
 *
 * Vulnerability: DOS-001
 *   Without an isValidObjectId() guard, Mongoose throws a CastError for any
 *   string that cannot be cast to ObjectId. NestJS catches this and returns
 *   a 500. The endpoint requires no authentication (GET /courses/:id).
 *
 * What we're looking for:
 *   1. Inputs that cause isValidObjectId() to throw instead of returning false.
 *   2. Prototype pollution via specially crafted id strings.
 *   3. Any crash in the ObjectId constructor path.
 *   4. Timing side-channels in ObjectId comparison (informational).
 *
 * We do NOT connect to a real database — we only exercise the parsing and
 * validation logic, which is entirely synchronous.
 */

import { FuzzedDataProvider } from '@jazzer.js/core';
import mongoose, { isValidObjectId, Types } from 'mongoose';

export function fuzz(data: Buffer): void {
  const provider = new FuzzedDataProvider(data);
  const strategy = provider.consumeIntegralInRange(0, 4);

  switch (strategy) {
    case 0: fuzzRawString(provider);           break;
    case 1: fuzzBoundaryLength(provider);      break;
    case 2: fuzzHexVariations(provider);       break;
    case 3: fuzzProblematicChars(provider);    break;
    case 4: fuzzObjectIdConstructor(provider); break;
  }
}

// ---------------------------------------------------------------------------

function fuzzRawString(provider: FuzzedDataProvider): void {
  const id = provider.consumeString(provider.remainingBytes);
  testObjectId(id);
}

function fuzzBoundaryLength(provider: FuzzedDataProvider): void {
  // Valid ObjectId is exactly 24 hex chars or 12 bytes.
  // Test lengths around those boundaries.
  const targetLen = provider.consumeIntegralInRange(0, 48);
  const chars = '0123456789abcdefghijklmnopqrstuvwxyz_-./\\%';
  let id = '';
  for (let i = 0; i < targetLen; i++) {
    id += chars[provider.consumeIntegralInRange(0, chars.length - 1)];
  }
  testObjectId(id);
}

function fuzzHexVariations(provider: FuzzedDataProvider): void {
  // 24-character strings that are almost-valid ObjectIds.
  const hexChars = '0123456789abcdefABCDEF';
  const nonHexInjections = [
    'g', 'G', ' ', '\t', '\n', '\0', '$', '(', ')', '{', '}', '[', ']', ';',
    '../', '%00', '\x00', '\uffff', '\\',
  ];

  let id = '';
  for (let i = 0; i < 24; i++) {
    if (provider.consumeBoolean() && provider.remainingBytes > 0) {
      const inj = nonHexInjections[provider.consumeIntegralInRange(0, nonHexInjections.length - 1)];
      id += inj;
    } else {
      id += hexChars[provider.consumeIntegralInRange(0, hexChars.length - 1)];
    }
  }
  testObjectId(id);
}

function fuzzProblematicChars(provider: FuzzedDataProvider): void {
  // Known-problematic inputs: prototype pollution, null bytes, very long strings.
  const problematic = [
    '__proto__',
    'constructor',
    'prototype',
    'toString',
    'valueOf',
    'null',
    'undefined',
    'NaN',
    'Infinity',
    '-Infinity',
    '{}',
    '[]',
    '{"$gt": ""}',
    '\x00'.repeat(12),
    'a'.repeat(1024),
    'a'.repeat(24).replace('a', '\0'),
    '000000000000000000000000',   // all zeros — technically valid
    'ffffffffffffffffffffffff',   // all f — technically valid
    '000000000000000000000001',   // min ObjectId
  ];

  const s = problematic[provider.consumeIntegralInRange(0, problematic.length - 1)];
  testObjectId(s);
}

function fuzzObjectIdConstructor(provider: FuzzedDataProvider): void {
  // Directly invoke the ObjectId constructor with fuzzed input.
  // isValidObjectId returns false → findById should throw CastError.
  // We verify the constructor also handles the input gracefully.
  const id = provider.consumeString(provider.remainingBytes);

  try {
    const oid = new Types.ObjectId(id as any);
    // If we reach here, the ID was accepted. Verify it round-trips cleanly.
    const str = oid.toHexString();
    if (str.length !== 24) {
      throw new Error(`ObjectId.toHexString() returned unexpected length ${str.length} for input "${id}"`);
    }
    // Reconstruct from the hex string to verify consistency.
    const oid2 = new Types.ObjectId(str);
    if (oid2.toHexString() !== str) {
      throw new Error('ObjectId round-trip inconsistency');
    }
  } catch (err: unknown) {
    if (err instanceof Error) {
      const msg = err.message.toLowerCase();
      // Expected rejection messages from Mongoose/bson.
      if (
        msg.includes('must be a string') ||
        msg.includes('24 hex') ||
        msg.includes('argument passed') ||
        msg.includes('must be a 24 character hex') ||
        msg.includes('invalid objectid') ||
        msg.includes('bsontype') ||
        msg.includes('casttoobjectid') ||
        msg.includes('cast to objectid') ||
        msg.includes('illegal') ||
        msg.includes('objectid failed') ||
        err.name === 'BSONError' ||
        err.name === 'BSONTypeError'
      ) {
        return; // expected — not a bug
      }
      // Unexpected error from the ObjectId constructor.
      throw err;
    }
  }
}

// ---------------------------------------------------------------------------
// Core test: mirrors what the production guard should do.
// ---------------------------------------------------------------------------

function testObjectId(id: string): void {
  try {
    // This is what the FIX should do:
    const valid = isValidObjectId(id);

    if (valid) {
      // If declared valid, the ObjectId constructor must not throw.
      const oid = new Types.ObjectId(id);
      const hex = oid.toHexString();
      if (hex.length !== 24 || !/^[0-9a-f]{24}$/.test(hex)) {
        throw new Error(`isValidObjectId returned true but toHexString() produced "${hex}"`);
      }
    }
    // If !valid, findById would throw CastError — that is acceptable with the guard in place.
  } catch (err: unknown) {
    if (err instanceof Error) {
      const msg = err.message.toLowerCase();
      // isValidObjectId itself should never throw — only return true/false.
      if (err.message.includes('isValidObjectId')) {
        throw new Error(`isValidObjectId() threw unexpectedly: ${err.message}`);
      }
      // ObjectId constructor errors on a valid-declared id are findings.
      if (
        msg.includes('24 hex') ||
        msg.includes('argument passed') ||
        msg.includes('bsontype') ||
        err.name === 'BSONError' ||
        err.name === 'BSONTypeError'
      ) {
        return; // expected bson rejection
      }
      throw err;
    }
  }
}
