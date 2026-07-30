# Fuzzing Infrastructure — Temene / SeNest

This directory contains production-quality [Jazzer.js](https://github.com/CodeIntelligenceTesting/jazzer.js)
harnesses for the SeNest NestJS backend, deterministic proof-of-concept scripts for each
confirmed vulnerability, seed corpora, mutation dictionaries, and a ClusterFuzzLite
configuration for continuous fuzzing in CI.

---

## Prerequisites

```bash
# 1. Install harness dependencies (separate from the main app)
cd fuzz
npm install

# 2. Jazzer.js requires Node.js >= 16 and a build of libFuzzer.
#    On Linux/macOS with clang:
npm install -g @jazzer.js/core   # installs the jazzer CLI globally
```

---

## Running a Single Harness

```bash
# From the fuzz/ directory

# JWT verify fuzzer (fast, no DB dependency)
npm run fuzz:jwt

# NoSQL query-transform fuzzer
npm run fuzz:query

# File-extension fuzzer
npm run fuzz:fileext

# DTO class-validator fuzzer
npm run fuzz:dto

# MongoDB ObjectId parse fuzzer
npm run fuzz:objectid

# bcrypt hash+compare fuzzer
npm run fuzz:bcrypt

# Full login flow (uses in-memory MongoDB — slower)
npm run fuzz:login
```

Each command runs until Jazzer.js finds a crash or you press Ctrl+C.  
Findings are written as minimized inputs to `fuzz/findings/<harness-name>/`.

---

## Reproducing a Crash

When Jazzer.js finds a crash it prints the path to the crashing input, e.g.:

```
artifact_prefix='./'; Test unit written to ./crash-abc123
```

Reproduce deterministically:

```bash
jazzer fuzz/harnesses/02-query-transform.fuzz.ts ./crash-abc123
```

---

## Running PoCs (Deterministic Vulnerability Reproductions)

Each PoC is a standalone TypeScript script that exercises a confirmed vulnerability
without fuzzing — useful for CI regression tests.

```bash
npm run poc:nosql      # NOSQLI-001 — NoSQL operator injection
npm run poc:nullderef  # NPE-001    — null dereference in getUser
npm run poc:jwtsecret  # CONFIG-001 — undefined JWT secret
npm run poc:unauthed   # AUTH-001   — unauthorized course deletion
npm run poc:objectid   # DOS-001    — CastError via invalid ObjectId
```

---

## Harness Overview

| File | Target | DB needed |
|------|--------|-----------|
| `01-jwt-verify.fuzz.ts` | `jsonwebtoken.verify()` | No |
| `02-query-transform.fuzz.ts` | `getAllCourses` query pipeline | No |
| `03-file-extension.fuzz.ts` | `path.extname()` + filename sanitization | No |
| `04-dto-validation.fuzz.ts` | `class-validator` DTO validation | No |
| `05-objectid-parse.fuzz.ts` | Mongoose `isValidObjectId` + `findById` | No |
| `06-bcrypt-hashpwd.fuzz.ts` | `bcrypt.hash` / `bcrypt.compare` | No |
| `07-login-logic.fuzz.ts` | Full `AuthService.login` path | Yes (in-memory) |

---

## Corpus and Dictionaries

```
corpus/
  01-jwt-verify/      — valid JWTs, truncated tokens, alg:none examples
  02-query-transform/ — normal + adversarial query strings
  03-file-extension/  — normal filenames + traversal attempts
  04-dto-validation/  — valid + boundary-condition DTO JSON
  05-objectid-parse/  — valid ObjectIds + garbage strings
  06-bcrypt-hashpwd/  — short/long/unicode passwords + malformed hash strings
  07-login-logic/     — full login request bodies

dicts/
  jwt.dict       — JWT structural tokens (header.payload.sig, alg names)
  mongodb.dict   — MongoDB operator strings ($where, $ne, $regex, …)
  filenames.dict — Adversarial filename fragments
  general.dict   — Common injection strings (null bytes, unicode, etc.)
```

---

## ClusterFuzzLite (CI Continuous Fuzzing)

See `.clusterfuzzlite/` at the project root.

```bash
# Local build test (requires Docker)
docker build -t temene-fuzz -f .clusterfuzzlite/Dockerfile .
docker run --rm temene-fuzz bash /src/.clusterfuzzlite/build.sh
```

The GitHub Actions workflow at `.github/workflows/clusterfuzzlite.yml` runs fuzzing
on every PR and for 30 minutes on each push to `main`.
