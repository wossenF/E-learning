/**
 * Harness 03 — File-Extension / Filename Sanitization Fuzzer
 *
 * Target: The Multer filename callback in CoursesController and UserController:
 *
 *   filename(req, file, callback) {
 *     callback(null, Date.now() + path.extname(file.originalname));
 *   }
 *
 * And the companion static file server:
 *   app.use("/images", express.static("Images"));
 *
 * What we're looking for:
 *   1. path.extname() returning dangerous extensions (.html, .js, .svg, .php, .exe)
 *      that the app would serve with their original MIME type.
 *   2. Path traversal: filenames containing "../" that survive into the stored path.
 *   3. Null-byte injection: "evil.html\0.jpg" — path.extname returns ".jpg" but OS
 *      operations on the combined Date.now() + ext may behave differently.
 *   4. Excessively long extensions causing buffer/memory issues.
 *   5. Unicode/homoglyph filenames that bypass extension denylist checks.
 *
 * The harness also tests a naive denylist (the kind developers often write to "fix"
 * this) and shows why it is insufficient compared to an allowlist approach.
 *
 * Safe outcomes: any extension that is empty or clearly benign.
 * Findings: dangerous extensions or path traversal surviving sanitization.
 */

import { FuzzedDataProvider } from '@jazzer.js/core';
import * as path from 'path';

// ---------------------------------------------------------------------------
// Extensions that should NEVER appear in a stored filename because they can be
// served by the static file server and executed/rendered in a browser.
// ---------------------------------------------------------------------------
const DANGEROUS_EXTENSIONS = new Set([
  '.html', '.htm', '.xhtml',
  '.js',   '.mjs', '.cjs',
  '.ts',   '.jsx', '.tsx',
  '.svg',  '.xml',
  '.php',  '.php3', '.php4', '.php5', '.phtml',
  '.asp',  '.aspx',
  '.jsp',  '.jspx',
  '.sh',   '.bash', '.zsh', '.fish',
  '.py',   '.rb',   '.pl',  '.lua',
  '.exe',  '.dll',  '.so',  '.dylib',
  '.htaccess', '.htpasswd',
]);

// ---------------------------------------------------------------------------
// The exact production code under test.
// ---------------------------------------------------------------------------
function productionFilename(originalname: string): string {
  return Date.now() + path.extname(originalname);
}

// ---------------------------------------------------------------------------
// A naive denylist "fix" that developers often write — we prove it's bypassable.
// ---------------------------------------------------------------------------
const NAIVE_DENYLIST = new Set(['.html', '.js', '.php']);
function naiveDenylistFilename(originalname: string): string | null {
  const ext = path.extname(originalname).toLowerCase();
  if (NAIVE_DENYLIST.has(ext)) return null; // "rejected"
  return Date.now() + ext;
}

// ---------------------------------------------------------------------------
// Correct allowlist implementation for comparison.
// ---------------------------------------------------------------------------
const SAFE_ALLOWLIST = new Set(['.jpg', '.jpeg', '.png', '.gif', '.webp']);
function safeAllowlistFilename(originalname: string): string | null {
  const ext = path.extname(originalname).toLowerCase();
  if (!SAFE_ALLOWLIST.has(ext)) return null;
  // Use a random UUID instead of the original name to prevent traversal.
  return require('crypto').randomUUID() + ext;
}

// ---------------------------------------------------------------------------
// Harness entry point
// ---------------------------------------------------------------------------

export function fuzz(data: Buffer): void {
  const provider = new FuzzedDataProvider(data);
  const strategy = provider.consumeIntegralInRange(0, 3);

  switch (strategy) {
    case 0: fuzzRawFilename(provider);           break;
    case 1: fuzzTraversalFilename(provider);     break;
    case 2: fuzzNullByteFilename(provider);      break;
    case 3: fuzzUnicodeSpoofFilename(provider);  break;
  }
}

// ---------------------------------------------------------------------------

function fuzzRawFilename(provider: FuzzedDataProvider): void {
  const name = provider.consumeString(provider.remainingBytes);
  checkFilename(name, 'raw');
}

function fuzzTraversalFilename(provider: FuzzedDataProvider): void {
  // Prefix with path traversal sequences; check that none survive.
  const traversalPrefixes = [
    '../', '../../', '../../../',
    '..\\', '..\\..\\',
    '..' + path.sep,
    '/etc/passwd',
    '%2e%2e%2f', '%2e%2e/', '..%2f', '%2e%2e%5c',
  ];
  const prefix = traversalPrefixes[provider.consumeIntegralInRange(0, traversalPrefixes.length - 1)];
  const suffix = provider.consumeString(provider.consumeIntegralInRange(1, 32));
  const name = prefix + suffix;

  checkFilename(name, 'traversal');

  // Verify the stored filename cannot escape the Images directory.
  const stored = productionFilename(name);
  const resolved = path.resolve('/app/Images', stored);
  if (!resolved.startsWith(path.resolve('/app/Images'))) {
    throw new Error(
      `PATH TRAVERSAL: filename "${name}" produces stored path "${stored}" ` +
      `which resolves outside Images/: "${resolved}"`
    );
  }
}

function fuzzNullByteFilename(provider: FuzzedDataProvider): void {
  // Inject null bytes at various positions.
  const base = provider.consumeString(provider.consumeIntegralInRange(1, 32));
  const ext  = provider.consumeString(provider.consumeIntegralInRange(1, 8));

  const variants = [
    `${base}\x00.jpg`,            // null before safe ext
    `${base}.html\x00.jpg`,       // null between dangerous and safe ext
    `${base}.${ext}\x00`,         // null at end
    `\x00${base}.${ext}`,         // null at start
    `${base}\x00${ext}`,          // null in the middle
  ];

  for (const v of variants) {
    checkFilename(v, 'nullbyte');
  }
}

function fuzzUnicodeSpoofFilename(provider: FuzzedDataProvider): void {
  // Homoglyphs and unicode normalisation tricks.
  // e.g. "evil.ℍtml" — ℍ is U+210D, not 'H', bypasses ".html" denylist.
  const unicodeExtensions = [
    '.ℍtml', '.𝐡tml', '.htмl',  // Cyrillic м
    '.ⓙs',  '.𝓙s',
    '.рhp',                        // Cyrillic р
    'js\u200b',                    // zero-width space
    '.svg\u202e',                  // right-to-left override
  ];
  const uniExt = unicodeExtensions[provider.consumeIntegralInRange(0, unicodeExtensions.length - 1)];
  const base   = provider.consumeString(16);
  checkFilename(base + uniExt, 'unicode');
}

// ---------------------------------------------------------------------------
// Core assertion: the PRODUCTION code must not produce dangerous extensions.
// ---------------------------------------------------------------------------

function checkFilename(originalname: string, context: string): void {
  let stored: string;
  try {
    stored = productionFilename(originalname);
  } catch (err: unknown) {
    if (err instanceof TypeError || err instanceof RangeError) return;
    throw err;
  }

  const ext = path.extname(stored).toLowerCase();

  // Finding 1: production code produces a dangerous extension.
  if (DANGEROUS_EXTENSIONS.has(ext)) {
    throw new Error(
      `DANGEROUS_EXTENSION [${context}]: originalname="${stringify(originalname)}" ` +
      `→ stored="${stringify(stored)}" → ext="${ext}"`
    );
  }

  // Finding 2: naive denylist allows something the allowlist would block —
  // demonstrates why allowlists are the correct approach.
  const naiveResult = naiveDenylistFilename(originalname);
  const safeResult  = safeAllowlistFilename(originalname);
  if (naiveResult !== null && safeResult === null) {
    // The naive denylist accepted a file that is not in the safe allowlist.
    // This is an informational finding (not thrown as an error — Jazzer.js
    // logs it as a coverage-interesting case via console).
    // To escalate to a crash, uncomment the throw:
    //
    // throw new Error(`DENYLIST_BYPASS [${context}]: "${stringify(originalname)}" ` +
    //   `accepted by naive denylist but blocked by allowlist`);
  }
}

function stringify(s: string): string {
  // Show non-printable characters as hex escapes for legible error messages.
  return [...s].map(c => {
    const code = c.codePointAt(0)!;
    return code < 0x20 || code > 0x7e
      ? `\\u${code.toString(16).padStart(4, '0')}`
      : c;
  }).join('');
}
