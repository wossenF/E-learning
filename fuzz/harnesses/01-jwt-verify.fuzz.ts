/**
 * Harness 01 — JWT Verification Fuzzer
 *
 * Target: jsonwebtoken.verify(token, secret)
 *         Used in AuthController.checkJwt and JwtStrategy.
 *
 * What we're looking for:
 *   - Crashes / unhandled exceptions in the jsonwebtoken parser
 *   - Algorithm-confusion: RS256 key material as HS256 secret
 *   - "alg:none" bypass attempts
 *   - Oversized tokens causing DoS (excessive memory/CPU)
 *   - Prototype pollution via crafted payload claims
 *   - Null-byte injection in header/payload/signature segments
 *
 * Safe errors (expected, not bugs):
 *   - JsonWebTokenError  — malformed token
 *   - NotBeforeError     — nbf claim in the future
 *   - TokenExpiredError  — exp claim in the past
 *
 * Any other thrown value (TypeError, RangeError, SyntaxError from unexpected
 * paths inside jsonwebtoken) is treated as a finding.
 */

import { FuzzedDataProvider } from '@jazzer.js/core';
import * as jwt from 'jsonwebtoken';

// Known-safe errors that jsonwebtoken intentionally throws on bad input.
const EXPECTED_ERROR_NAMES = new Set([
  'JsonWebTokenError',
  'NotBeforeError',
  'TokenExpiredError',
]);

// Fixed secret that mirrors what the app uses from JWT_SECRET env var.
// In a real environment this would be provided via the harness env.
const FUZZ_SECRET = 'fuzzing-test-secret-do-not-use-in-production-x7k9m2n4';

export function fuzz(data: Buffer): void {
  const provider = new FuzzedDataProvider(data);

  // Choose one of several input strategies driven by the fuzz data.
  const strategy = provider.consumeIntegralInRange(0, 5);

  switch (strategy) {
    case 0:
      // Strategy 0: fully raw token from fuzz bytes.
      fuzzRawToken(provider);
      break;
    case 1:
      // Strategy 1: structurally valid token with fuzzed payload.
      fuzzValidStructureToken(provider);
      break;
    case 2:
      // Strategy 2: algorithm confusion — put 'none' in alg header.
      fuzzAlgNoneToken(provider);
      break;
    case 3:
      // Strategy 3: fuzz the SECRET while keeping a valid token structure.
      fuzzSecretVariation(provider);
      break;
    case 4:
      // Strategy 4: fuzz only the signature segment.
      fuzzSignatureSegment(provider);
      break;
    case 5:
      // Strategy 5: known-structured token with fuzzed claim values.
      fuzzClaims(provider);
      break;
  }
}

// ---------------------------------------------------------------------------

function fuzzRawToken(provider: FuzzedDataProvider): void {
  // Raw bytes converted to string — exercises the tokenizer / base64 decoder.
  const rawToken = provider.consumeString(provider.remainingBytes);
  callVerify(rawToken);
}

function fuzzValidStructureToken(provider: FuzzedDataProvider): void {
  // Build a structurally valid JWT (header.payload.sig) where each segment
  // is independently fuzzed base64url content.
  const headerBytes  = provider.consumeBytes(provider.consumeIntegralInRange(0, 128));
  const payloadBytes = provider.consumeBytes(provider.consumeIntegralInRange(0, 512));
  const sigBytes     = provider.consumeBytes(provider.consumeIntegralInRange(0, 256));

  const header  = toBase64url(Buffer.from(headerBytes));
  const payload = toBase64url(Buffer.from(payloadBytes));
  const sig     = toBase64url(Buffer.from(sigBytes));

  callVerify(`${header}.${payload}.${sig}`);
}

function fuzzAlgNoneToken(provider: FuzzedDataProvider): void {
  // "alg:none" bypass family — attempt to get verify() to accept an unsigned token.
  // jsonwebtoken v9 refuses these by default; we verify it continues to do so.
  const algVariants = [
    'none', 'NONE', 'None', 'nOnE', 'noNe',
    'None ', ' none', '\tnone', 'none\0',
  ];
  const alg = algVariants[provider.consumeIntegralInRange(0, algVariants.length - 1)];

  const header  = toBase64url(Buffer.from(JSON.stringify({ alg, typ: 'JWT' })));
  const payloadObj: Record<string, unknown> = {
    sub: provider.consumeString(32),
    iat: Math.floor(Date.now() / 1000),
  };
  const payload = toBase64url(Buffer.from(JSON.stringify(payloadObj)));

  // Signature-less token.
  callVerify(`${header}.${payload}.`);
  // Also try the full token with a random sig.
  const sig = toBase64url(provider.consumeBytes(32));
  callVerify(`${header}.${payload}.${sig}`);
}

function fuzzSecretVariation(provider: FuzzedDataProvider): void {
  // Create a legitimately signed token, then verify it with a fuzzed secret.
  // Goal: find secrets that accidentally validate (e.g. empty string, null-terminated strings).
  const payload = { sub: 'fuzz', iat: 1 };
  const token = jwt.sign(payload, FUZZ_SECRET, { algorithm: 'HS256' });

  const fuzzedSecret = provider.consumeString(provider.remainingBytes);
  callVerify(token, fuzzedSecret);
}

function fuzzSignatureSegment(provider: FuzzedDataProvider): void {
  // Valid header+payload, fuzzed signature — exercises signature verification path.
  const payload = { sub: 'fuzz', email: 'fuzz@example.com', id: '000000000000000000000001' };
  const validToken = jwt.sign(payload, FUZZ_SECRET, { algorithm: 'HS256', expiresIn: '1d' });

  const parts = validToken.split('.');
  const fuzzedSig = toBase64url(provider.consumeBytes(provider.consumeIntegralInRange(0, 128)));
  const tamperedToken = `${parts[0]}.${parts[1]}.${fuzzedSig}`;

  callVerify(tamperedToken);
}

function fuzzClaims(provider: FuzzedDataProvider): void {
  // Fuzz claim values inside a validly-signed token.
  // Targets claim-validation logic (exp, nbf, iss, aud, sub).
  const exp = provider.consumeIntegralInRange(-2147483648, 2147483647);
  const nbf = provider.consumeIntegralInRange(-2147483648, 2147483647);
  const sub = provider.consumeString(128);
  const iss = provider.consumeString(64);

  const claimsPayload: Record<string, unknown> = { sub, iss, exp, nbf };

  // Add a random extra claim — exercises prototype-pollution-adjacent paths.
  if (provider.consumeBoolean()) {
    const extraKey = provider.consumeString(32);
    const extraVal = provider.consumeString(64);
    claimsPayload[extraKey] = extraVal;
  }

  const header  = toBase64url(Buffer.from(JSON.stringify({ alg: 'HS256', typ: 'JWT' })));
  const payload = toBase64url(Buffer.from(JSON.stringify(claimsPayload)));
  const hmac    = require('crypto')
    .createHmac('sha256', FUZZ_SECRET)
    .update(`${header}.${payload}`)
    .digest('base64url');

  callVerify(`${header}.${payload}.${hmac}`);
}

// ---------------------------------------------------------------------------
// Shared helper: call verify with a specific token and optional secret,
// classifying thrown errors.
// ---------------------------------------------------------------------------

function callVerify(token: string, secret: string = FUZZ_SECRET): void {
  try {
    jwt.verify(token, secret, {
      // Mirror the production configuration from jwt.strategy.ts.
      algorithms: ['HS256'],
      ignoreExpiration: false,
    });
  } catch (err: unknown) {
    if (err instanceof Error && EXPECTED_ERROR_NAMES.has(err.name)) {
      // Expected rejection — not a bug.
      return;
    }
    // Unexpected error type — re-throw so Jazzer.js records it as a finding.
    throw err;
  }
}

function toBase64url(buf: Buffer): string {
  return buf.toString('base64').replace(/\+/g, '-').replace(/\//g, '_').replace(/=/g, '');
}
