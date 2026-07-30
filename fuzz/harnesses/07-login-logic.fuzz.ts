/**
 * Harness 07 — Full Login Flow Fuzzer (with in-memory MongoDB)
 *
 * Target: AuthService.login — the complete path including:
 *   - UserService.getUser (findOne + populate)  ← NPE-001 if user not found
 *   - bcrypt.hash(password, user.salt)           ← CRYPTO-001
 *   - string comparison of hashes
 *   - JwtService.sign(payload)
 *
 * This harness:
 *   1. Spins up mongodb-memory-server once (beforeAll equivalent).
 *   2. Pre-seeds one real user.
 *   3. For each fuzz iteration, calls the login logic with fuzzed credentials.
 *   4. Also fuzzes the getUser path directly with arbitrary email strings.
 *
 * What we're looking for:
 *   - NPE-001: getUser crashing when user not found (null.populate()).
 *   - Authentication bypass: wrong password accepted.
 *   - Crashes from unexpected input shapes.
 *   - JWT signing with malformed payload fields.
 *
 * NOTE: This harness is slower than the others because it spawns a real MongoDB
 * process.  Run it with a lower iteration budget:
 *   jazzer fuzz/harnesses/07-login-logic.fuzz.ts --runs=50000
 */

import { FuzzedDataProvider } from '@jazzer.js/core';
import * as bcrypt from 'bcrypt';
import * as jwt from 'jsonwebtoken';
import { MongoMemoryServer } from 'mongodb-memory-server';
import mongoose, { Schema, Model, Document } from 'mongoose';

// ---------------------------------------------------------------------------
// Mongoose schema / model — mirroring SeNest/src/user/Schemas/user.schema.ts
// ---------------------------------------------------------------------------

interface IUser extends Document {
  first_name: string;
  last_name: string;
  email: string;
  password: string;
  salt: string;
  profile_picture: string;
  student: mongoose.Types.ObjectId[];
  courses_enrolled: mongoose.Types.ObjectId[];
  courses_created: mongoose.Types.ObjectId[];
}

const UserSchema = new Schema<IUser>({
  first_name: { type: String, required: true },
  last_name:  { type: String },
  email:      { type: String, unique: true },
  password:   { type: String, required: true },
  salt:       { type: String },
  profile_picture: { type: String },
  student:          [{ type: Schema.Types.ObjectId, ref: 'Users' }],
  courses_enrolled: [{ type: Schema.Types.ObjectId, ref: 'Courses' }],
  courses_created:  [{ type: Schema.Types.ObjectId, ref: 'Courses' }],
}, { timestamps: true });

// ---------------------------------------------------------------------------
// Module-level state (initialized once, reused across fuzz iterations).
// Jazzer.js calls fuzz() repeatedly in the same process.
// ---------------------------------------------------------------------------

let mongod:    MongoMemoryServer | null = null;
let UserModel: Model<IUser> | null = null;

const SEED_EMAIL    = 'seed@fuzz.test';
const SEED_PASSWORD = 'SeedPassword123!';
const JWT_SECRET    = 'harness-test-secret-32chars-xxxxxxx';

let seedSalt: string;
let seedHash: string;

// ---------------------------------------------------------------------------
// Lifecycle helpers
// ---------------------------------------------------------------------------

async function ensureInit(): Promise<void> {
  if (mongod !== null) return;

  mongod = await MongoMemoryServer.create();
  const uri = mongod.getUri();
  await mongoose.connect(uri);

  // Register model only once.
  UserModel = mongoose.models['Users'] as Model<IUser> ||
              mongoose.model<IUser>('Users', UserSchema);

  // Seed one known-good user.
  seedSalt = await bcrypt.genSalt(10);
  seedHash = await bcrypt.hash(SEED_PASSWORD, seedSalt);

  await UserModel.create({
    first_name: 'Seed',
    last_name: 'User',
    email: SEED_EMAIL,
    password: seedHash,
    salt: seedSalt,
    profile_picture: '',
  });
}

// ---------------------------------------------------------------------------
// Replica of the login logic (AuthService.login + UserService.getUser).
// ---------------------------------------------------------------------------

async function getUser(email: string): Promise<IUser | null> {
  if (!UserModel) throw new Error('UserModel not initialized');
  // BUG: the production code does `.populate()` directly on the result without
  // checking for null.  We exercise BOTH the buggy path and the fixed path.
  const user = await UserModel.findOne({ email });
  return user;
}

async function loginVulnerable(email: string, password: string): Promise<{ accessToken: string } | null> {
  // Replicates the EXACT vulnerable code from auth.service.ts:
  //   const user: any = await this.usersService.getUser(email);
  //   if (!user) { throw new NotFoundException('User Not Found'); }
  //
  // In the production code, getUser is:
  //   return await (await this.usersModel.findOne({ email })).populate(...)
  //   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  //   This throws TypeError if findOne returns null.

  const user = await UserModel!.findOne({ email }) as any;

  // VULNERABLE PATH: production calls .populate() without null check.
  // We intentionally reproduce the bug here to confirm Jazzer.js detects it.
  let populatedUser: any;
  try {
    // This is the line from production (user.service.ts:16).
    populatedUser = await (user as any).populate(['student', 'courses_enrolled', 'courses_created']);
  } catch (err: unknown) {
    if (
      err instanceof TypeError &&
      (err.message.includes('Cannot read') || err.message.includes('populate'))
    ) {
      throw new Error(
        `NPE_001_CONFIRMED: getUser("${escapeStr(email)}") — ` +
        `findOne returned null, .populate() threw: ${err.message}`
      );
    }
    throw err;
  }

  if (!populatedUser) return null;

  // Password verification (CRYPTO-001 pattern).
  const hashedPwd = await bcrypt.hash(password, populatedUser.salt);
  if (hashedPwd !== populatedUser.password) return null;

  // JWT sign.
  const payload = { email: populatedUser.email, id: populatedUser.id };
  const accessToken = jwt.sign(payload, JWT_SECRET, { algorithm: 'HS256', expiresIn: '1d' });
  return { accessToken };
}

// ---------------------------------------------------------------------------
// Harness entry point
// ---------------------------------------------------------------------------

export async function fuzz(data: Buffer): Promise<void> {
  await ensureInit();

  const provider = new FuzzedDataProvider(data);
  const strategy = provider.consumeIntegralInRange(0, 3);

  switch (strategy) {
    case 0:
      // Fuzz email — exercises the NPE-001 path (non-existent user).
      await fuzzEmailOnly(provider);
      break;
    case 1:
      // Fuzz credentials against the seeded user — tests bypass paths.
      await fuzzCredentials(provider);
      break;
    case 2:
      // Fuzz with a valid email but malformed password data types.
      await fuzzPasswordType(provider);
      break;
    case 3:
      // Fuzz getUser with emails that look like MongoDB operators.
      await fuzzOperatorEmail(provider);
      break;
  }
}

// ---------------------------------------------------------------------------

async function fuzzEmailOnly(provider: FuzzedDataProvider): Promise<void> {
  const email = provider.consumeString(provider.remainingBytes);
  try {
    await loginVulnerable(email, 'doesnotmatter');
  } catch (err: unknown) {
    if (err instanceof Error && err.message.startsWith('NPE_001_CONFIRMED')) {
      throw err; // Surface as a Jazzer.js finding.
    }
    // Other errors (NotFoundException equivalent, etc.) are expected.
  }
}

async function fuzzCredentials(provider: FuzzedDataProvider): Promise<void> {
  // Use the real seeded email; fuzz only the password.
  const password = provider.consumeString(provider.consumeIntegralInRange(0, 200));
  try {
    const result = await loginVulnerable(SEED_EMAIL, password);
    if (result !== null) {
      // Authentication succeeded with a fuzzed password — check it's the right one.
      if (password !== SEED_PASSWORD) {
        // Hashes matched for a wrong password — authentication bypass!
        throw new Error(
          `AUTH_BYPASS: login succeeded with fuzzed password "${escapeStr(password)}" ` +
          `(correct password is "${escapeStr(SEED_PASSWORD)}")`
        );
      }
    }
  } catch (err: unknown) {
    if (err instanceof Error && (err.message.startsWith('NPE_001') || err.message.startsWith('AUTH_BYPASS'))) {
      throw err;
    }
    // bcrypt / JWT errors on bad input are expected.
  }
}

async function fuzzPasswordType(provider: FuzzedDataProvider): Promise<void> {
  // Pass non-string values as password to stress bcrypt.hash type checks.
  const nonStringPasswords: unknown[] = [
    null, undefined, 0, -1, NaN, Infinity, [], {}, true, false,
    { password: provider.consumeString(16) },
    provider.consumeBytes(32),
  ];

  const pw = nonStringPasswords[provider.consumeIntegralInRange(0, nonStringPasswords.length - 1)];
  try {
    await loginVulnerable(SEED_EMAIL, pw as string);
  } catch (err: unknown) {
    if (err instanceof Error && err.message.startsWith('NPE_001')) throw err;
    // Type errors, bcrypt errors expected.
  }
}

async function fuzzOperatorEmail(provider: FuzzedDataProvider): Promise<void> {
  // MongoDB findOne({ email }) — if email is an object with operators it becomes an injection.
  // In practice NestJS gives us a string, but we test the model layer directly.
  const operatorEmails = [
    { $gt: '' },
    { $ne: null },
    { $regex: '.*' },
    { $where: 'this.email.length > 0' },
    provider.consumeString(32),
  ];

  const email = operatorEmails[provider.consumeIntegralInRange(0, operatorEmails.length - 1)];
  try {
    // Direct model call — bypasses the string type annotation.
    const user = await UserModel!.findOne({ email: email as any });
    if (user && typeof email === 'object') {
      throw new Error(
        `NOSQL_INJECTION_IN_GETUSER: findOne accepted operator object as email ` +
        `and returned user "${user.email}"`
      );
    }
  } catch (err: unknown) {
    if (err instanceof Error && err.message.startsWith('NOSQL_INJECTION')) throw err;
    // Cast errors, validation errors expected.
  }
}

// ---------------------------------------------------------------------------

function escapeStr(s: string): string {
  if (typeof s !== 'string') return String(s);
  return [...s].map(c => {
    const code = c.codePointAt(0)!;
    return code < 0x20 || code > 0x7e ? `\\x${code.toString(16).padStart(2, '0')}` : c;
  }).join('').slice(0, 60);
}
