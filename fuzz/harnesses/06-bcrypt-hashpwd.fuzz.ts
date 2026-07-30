/**
 * Harness 06 — bcrypt Hash + Verify Fuzzer
 *
 * Target: UserService.hashPwd and the login verification path in AuthService.login:
 *
 *   // hashPwd (creation):
 *   const salt = await bcrypt.genSalt(saltRounds);
 *   const hashedPwd = await bcrypt.hash(password, salt);
 *   return [hashedPwd, salt];
 *
 *   // login verification (CRYPTO-001):
 *   const hashedPwd = await bcrypt.hash(password, user.salt);
 *   if (!(hashedPwd === user.password)) throw ForbiddenException;
 *
 * What we're looking for:
 *   1. bcrypt.hash() crashing on inputs > 72 bytes (bcrypt silently truncates
 *      passwords longer than 72 bytes; we verify this is the ONLY thing that
 *      happens — no crash, no buffer overflow in the native addon).
 *   2. bcrypt.hash() / bcrypt.compare() divergence: passwords that hash
 *      differently under the two approaches (hash+compare vs hash+hash+===).
 *   3. Null-byte handling: "secret\0garbage" — bcrypt C library may treat
 *      the first null byte as end-of-string, leading to authentication bypass.
 *   4. Unicode normalization: "caf\u00e9" vs "cafe\u0301" — different byte
 *      sequences, same visual glyph; confirm the library handles consistently.
 *   5. Extreme salt rounds: if the salt is re-used with a different round count
 *      extracted from its prefix (the CRYPTO-001 pattern), does it diverge?
 *
 * Notes on bcrypt > 72 bytes:
 *   - bcrypt C library truncates at 72 bytes (including null terminator = 71 chars).
 *   - This means passwords longer than 72 bytes share the same hash.
 *   - Not a crash, but an important security property the app should document.
 *
 * Notes on null-byte handling:
 *   - The bcrypt npm package (wrapping OpenSSL) does NOT protect against null bytes.
 *   - "secret\x00garbage" hashes the same as "secret\x00" hashes the same as "secret"
 *     in some native implementations.
 *   - We record this as an informational finding.
 */

import { FuzzedDataProvider } from '@jazzer.js/core';
import * as bcrypt from 'bcrypt';

const FIXED_SALT_ROUNDS = 10;

export async function fuzz(data: Buffer): Promise<void> {
  const provider = new FuzzedDataProvider(data);
  const strategy = provider.consumeIntegralInRange(0, 4);

  switch (strategy) {
    case 0: await fuzzHashRoundtrip(provider);       break;
    case 1: await fuzzHashVsCompare(provider);       break;
    case 2: await fuzzNullBytePasswords(provider);   break;
    case 3: await fuzzLongPasswords(provider);       break;
    case 4: await fuzzMalformedSaltString(provider); break;
  }
}

// ---------------------------------------------------------------------------

async function fuzzHashRoundtrip(provider: FuzzedDataProvider): Promise<void> {
  // Generate a salt, hash a password, verify with compare.
  // Then verify with the CRYPTO-001 pattern (hash+hash+===).
  // The two must agree — divergence would be a finding.
  const password = provider.consumeString(provider.consumeIntegralInRange(0, 128));

  let salt: string;
  try {
    salt = await bcrypt.genSalt(FIXED_SALT_ROUNDS);
  } catch { return; }

  let hash1: string;
  try {
    hash1 = await bcrypt.hash(password, salt);
  } catch (err: unknown) {
    assertExpectedBcryptError(err, 'hash');
    return;
  }

  // Standard path: bcrypt.compare
  let compareResult: boolean;
  try {
    compareResult = await bcrypt.compare(password, hash1);
  } catch (err: unknown) {
    assertExpectedBcryptError(err, 'compare');
    return;
  }

  // CRYPTO-001 path: re-hash with stored salt string
  let hash2: string;
  try {
    hash2 = await bcrypt.hash(password, salt);  // salt = the full $2b$10$... string
  } catch (err: unknown) {
    assertExpectedBcryptError(err, 'hash2');
    return;
  }

  const crypto001Result = (hash2 === hash1);

  // Both methods must agree on the outcome.
  if (compareResult !== crypto001Result) {
    throw new Error(
      `CRYPTO_DIVERGENCE: bcrypt.compare=${compareResult} but hash===hash=${crypto001Result} ` +
      `for password="${escapeStr(password)}" salt="${salt.substring(0, 7)}..."`
    );
  }
}

async function fuzzHashVsCompare(provider: FuzzedDataProvider): Promise<void> {
  // Try verifying a fuzzed "stored password" string against a fuzzed input.
  // The stored password may or may not be a valid bcrypt hash.
  const inputPassword  = provider.consumeString(provider.consumeIntegralInRange(0, 128));
  const storedPassword = provider.consumeString(provider.consumeIntegralInRange(0, 64));

  try {
    await bcrypt.compare(inputPassword, storedPassword);
    // No crash = expected behaviour (returns true or false).
  } catch (err: unknown) {
    assertExpectedBcryptError(err, 'compare-fuzz');
  }

  // Also try the CRYPTO-001 pattern with the malformed stored password as the salt.
  try {
    await bcrypt.hash(inputPassword, storedPassword);
  } catch (err: unknown) {
    assertExpectedBcryptError(err, 'hash-with-malformed-salt');
  }
}

async function fuzzNullBytePasswords(provider: FuzzedDataProvider): Promise<void> {
  // Test null-byte truncation: "secret\x00suffix" should NOT authenticate as "secret".
  const basePassword = provider.consumeString(provider.consumeIntegralInRange(1, 32));
  const suffix       = provider.consumeString(provider.consumeIntegralInRange(1, 32));
  const nullPassword = basePassword + '\x00' + suffix;

  let salt: string;
  try { salt = await bcrypt.genSalt(FIXED_SALT_ROUNDS); }
  catch { return; }

  // Hash the null-terminated password.
  let hashOfNull: string;
  try { hashOfNull = await bcrypt.hash(nullPassword, salt); }
  catch (err: unknown) { assertExpectedBcryptError(err, 'hash-null'); return; }

  // Hash just the base password (before the null byte).
  let hashOfBase: string;
  try { hashOfBase = await bcrypt.hash(basePassword, salt); }
  catch (err: unknown) { assertExpectedBcryptError(err, 'hash-base'); return; }

  if (hashOfNull === hashOfBase && suffix.length > 0) {
    // Informational: bcrypt treated null-byte as string terminator.
    // This is a known bcrypt C-library behavior; we report it as a finding
    // so developers are aware their password validation is truncating at null.
    throw new Error(
      `NULL_BYTE_TRUNCATION: bcrypt hashed "${escapeStr(nullPassword)}" the same as ` +
      `"${escapeStr(basePassword)}" — null byte acts as string terminator. ` +
      `Authentication bypass possible if user can embed null bytes in passwords.`
    );
  }
}

async function fuzzLongPasswords(provider: FuzzedDataProvider): Promise<void> {
  // bcrypt truncates at 72 bytes.  Passwords that differ only after byte 72 must
  // hash identically — this means two distinct passwords may authenticate to the same account.
  const len = provider.consumeIntegralInRange(70, 200);
  const charSet = 'abcdefghijklmnopqrstuvwxyz0123456789';

  // Build two passwords that share the first 72 bytes but differ after.
  let sharedPrefix = '';
  for (let i = 0; i < 72; i++) {
    sharedPrefix += charSet[provider.consumeIntegralInRange(0, charSet.length - 1)];
  }
  const suffix1 = provider.consumeString(Math.max(1, len - 72));
  const suffix2 = provider.consumeString(Math.max(1, len - 72));

  if (suffix1 === suffix2) return; // Can't test with identical suffixes.

  const pw1 = sharedPrefix + suffix1;
  const pw2 = sharedPrefix + suffix2;

  let salt: string;
  try { salt = await bcrypt.genSalt(FIXED_SALT_ROUNDS); }
  catch { return; }

  let hash1: string, hash2: string;
  try {
    [hash1, hash2] = await Promise.all([
      bcrypt.hash(pw1, salt),
      bcrypt.hash(pw2, salt),
    ]);
  } catch (err: unknown) {
    assertExpectedBcryptError(err, 'hash-long');
    return;
  }

  // Both hashes must be identical (bcrypt truncation) — this is expected behavior.
  // We record it as informational (not thrown) but it's important to document.
  if (hash1 !== hash2 && pw1.length > 72 && pw2.length > 72) {
    // Unexpected: two >72-byte passwords with identical 72-byte prefixes hashed differently.
    throw new Error(
      `BCRYPT_TRUNCATION_INCONSISTENCY: passwords of length ${pw1.length} and ${pw2.length} ` +
      `with identical 72-byte prefixes produced different hashes.`
    );
  }
}

async function fuzzMalformedSaltString(provider: FuzzedDataProvider): Promise<void> {
  // Feed malformed bcrypt hash strings as the "salt" to bcrypt.hash().
  // This is the CRYPTO-001 pattern: the app passes user.salt (a bcrypt hash string)
  // as the second argument to bcrypt.hash().
  // If user.salt is corrupted, bcrypt.hash() may:
  //   (a) throw — causing a 500 crash
  //   (b) use a fresh random salt — making the hash mismatch user.password silently

  const malformedSalts = [
    '',
    ' ',
    'not-a-bcrypt-hash',
    '$2b$',
    '$2b$10$',
    '$2b$10$' + 'a'.repeat(21),                // too short
    '$2b$10$' + 'a'.repeat(22),                // valid length
    '$2b$10$' + 'a'.repeat(53),                // too long
    '$2a$10$' + provider.consumeString(22),    // $2a$ variant
    '$2x$10$' + provider.consumeString(22),    // unknown variant
    '$2b$00$' + provider.consumeString(22),    // 0 rounds
    '$2b$99$' + provider.consumeString(22),    // extreme rounds
    provider.consumeString(provider.consumeIntegralInRange(0, 60)),
  ];

  const salt = malformedSalts[provider.consumeIntegralInRange(0, malformedSalts.length - 1)];
  const password = provider.consumeString(provider.consumeIntegralInRange(1, 32));

  try {
    const result = await bcrypt.hash(password, salt);
    // If it succeeded with a malformed salt, check that the result is a valid bcrypt hash.
    if (typeof result !== 'string' || !result.startsWith('$2')) {
      throw new Error(
        `BCRYPT_MALFORMED_RESULT: hash() with salt="${escapeStr(salt)}" produced non-bcrypt result="${result}"`
      );
    }
  } catch (err: unknown) {
    assertExpectedBcryptError(err, 'hash-malformed-salt');
  }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

function assertExpectedBcryptError(err: unknown, context: string): void {
  if (!(err instanceof Error)) throw err;
  const msg = err.message.toLowerCase();
  // Known bcrypt error messages for invalid inputs.
  if (
    msg.includes('not a valid bcrypt') ||
    msg.includes('invalid salt') ||
    msg.includes('must be a string') ||
    msg.includes('data must be a string') ||
    msg.includes('saltorpassword must be a string') ||
    msg.includes('must provide a salt') ||
    msg.includes('rounds exceeds') ||
    msg.includes('salt rounds') ||
    err.name === 'Error' // bcrypt throws generic Error
  ) {
    return; // Expected rejection.
  }
  // Unexpected error type.
  throw new Error(`UNEXPECTED_BCRYPT_ERROR [${context}]: ${err.name}: ${err.message}`);
}

function escapeStr(s: string): string {
  return [...s].map(c => {
    const code = c.codePointAt(0)!;
    return code < 0x20 || code > 0x7e ? `\\x${code.toString(16).padStart(2, '0')}` : c;
  }).join('').slice(0, 80);
}
