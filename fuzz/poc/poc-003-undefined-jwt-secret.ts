/**
 * PoC-003 — CONFIG-001: Undefined JWT Secret
 *
 * Root cause (config/app.config.ts):
 *
 *   export default () => ({
 *     appSecret: process.env.JWT_SECRET,   // undefined if env var not set
 *   })
 *
 * If JWT_SECRET is not in the environment:
 *   1. passport-jwt's secretOrKey is undefined → first request to a protected
 *      route throws a hard-to-diagnose error.
 *   2. auth.controller.ts calls jwt.verify(token, undefined) → throws
 *      "secretOrPublicKey must have a value" from jsonwebtoken v9.
 *   3. jwt.sign(payload, undefined) produces a token, but it cannot be
 *      verified later because verification also requires a non-empty secret.
 *
 * We also test the edge case where JWT_SECRET = '' (empty string):
 *   - jwt.sign works with ''.
 *   - jwt.verify throws "secretOrPublicKey must have a value" in v9.
 *   - This means tokens signed with '' can never be verified by the app,
 *     breaking authentication entirely.
 *
 * Run: npx ts-node fuzz/poc/poc-003-undefined-jwt-secret.ts
 */

import * as jwt from 'jsonwebtoken';

function section(title: string): void {
  console.log('\n' + '─'.repeat(65));
  console.log(title);
  console.log('─'.repeat(65));
}

function trySign(secret: string | undefined, label: string): string | null {
  const payload = { email: 'test@example.com', id: '000000000000000000000001' };
  try {
    const token = jwt.sign(payload, secret as string, {
      algorithm: 'HS256',
      expiresIn: '1d',
    });
    console.log(`✓  jwt.sign with ${label}: succeeded → token starts with ${token.split('.')[0]}`);
    return token;
  } catch (err: unknown) {
    const msg = err instanceof Error ? err.message : String(err);
    console.log(`✗  jwt.sign with ${label}: THREW → ${msg}`);
    return null;
  }
}

function tryVerify(token: string | null, secret: string | undefined, label: string): void {
  if (token === null) {
    console.log(`   (skipping verify — sign failed for ${label})`);
    return;
  }
  try {
    const decoded = jwt.verify(token, secret as string, { algorithms: ['HS256'] });
    console.log(`✓  jwt.verify with ${label}: succeeded → decoded.email = ${(decoded as any).email}`);
  } catch (err: unknown) {
    const msg = err instanceof Error ? err.message : String(err);
    const name = err instanceof Error ? err.name : 'Error';
    console.log(`✗  jwt.verify with ${label}: THREW → [${name}] ${msg}`);
  }
}

(async () => {
  console.log('PoC-003 — Undefined JWT Secret');
  console.log('SECURITY.md reference: CONFIG-001');

  // --- Scenario 1: JWT_SECRET not set (undefined) ---
  section('Scenario 1: process.env.JWT_SECRET is undefined');

  const undefinedSecret: string | undefined = undefined;
  const token1 = trySign(undefinedSecret, 'undefined');
  tryVerify(token1, undefinedSecret, 'undefined');

  if (token1) {
    console.log('\n  ⚠  jwt.sign accepted undefined secret in some jsonwebtoken versions.');
    console.log('     Cross-verify: can another valid-secret verify this token?');
    tryVerify(token1, 'some-other-secret', 'a different secret');
    tryVerify(token1, 'undefined', 'the string "undefined"');
  }

  // --- Scenario 2: JWT_SECRET = '' (empty string) ---
  section('Scenario 2: process.env.JWT_SECRET = "" (empty string)');

  const emptySecret = '';
  const token2 = trySign(emptySecret, '""');
  tryVerify(token2, emptySecret, '""');

  // --- Scenario 3: Two different undefined-derived secrets cross-validating ---
  section('Scenario 3: Cross-validation — does a token signed with "undefined" verify with undefined?');

  // Simulate a restart where one process signed with undefined and another verifies.
  const signSecret: any = undefined;
  let crossToken: string | null = null;
  try {
    crossToken = jwt.sign({ email: 'victim@example.com' }, signSecret);
    console.log(`Signed with undefined: ${crossToken?.substring(0, 50)}…`);
  } catch (e) {
    console.log(`Could not sign with undefined: ${(e as Error).message}`);
  }

  if (crossToken) {
    // Try verifying with the empty string (another misconfiguration).
    tryVerify(crossToken, '', 'empty string after signing with undefined');
    tryVerify(crossToken, undefined, 'undefined after signing with undefined');
  }

  // --- Scenario 4: What the auth/check endpoint does ---
  section('Scenario 4: auth.controller.ts checkJwt behavior without JWT_SECRET');
  console.log('Production code: jwt.verify(token, this.configService.get("JWT_SECRET"))');
  console.log('If JWT_SECRET env var is missing, configService.get returns undefined.');

  const missingFromEnv: string = process.env.JWT_SECRET_NONEXISTENT as string; // undefined
  const testToken = 'eyJhbGciOiJIUzI1NiJ9.eyJlbWFpbCI6InRlc3RAZXhhbXBsZS5jb20ifQ.fake';
  tryVerify(testToken, missingFromEnv, 'configService.get("JWT_SECRET") → undefined');

  // --- Summary ---
  section('SUMMARY');
  console.log('Recommended fix: Validate JWT_SECRET at startup in main.ts:');
  console.log('');
  console.log('  const secret = process.env.JWT_SECRET;');
  console.log('  if (!secret || secret.length < 32) {');
  console.log('    console.error("FATAL: JWT_SECRET must be ≥32 characters");');
  console.log('    process.exit(1);');
  console.log('  }');
  console.log('');
  console.log('See SECURITY.md §CONFIG-001 for details.');
})();
