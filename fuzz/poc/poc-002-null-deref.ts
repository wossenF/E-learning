/**
 * PoC-002 — NPE-001: Null Dereference in UserService.getUser
 *
 * Root cause (user.service.ts:16):
 *
 *   async getUser(email: string): Promise<Users> {
 *     return await (await this.usersModel.findOne({ email })).populate(…);
 *                                                            ^^^^^^^^^^^
 *     If findOne returns null (user not found), .populate() throws:
 *       TypeError: Cannot read properties of null (reading 'populate')
 *   }
 *
 * This TypeError propagates through AuthService.login and up to
 * NestJS's global exception handler, which returns a 500 Internal Server
 * Error instead of a clean 404 NotFoundException.
 *
 * Impact:
 *   1. Crash returns a 500 with a stack trace (leaks internal paths in dev mode).
 *   2. Callers that expect a NotFoundException for flow control get unexpected errors.
 *   3. An attacker can identify valid vs. invalid email addresses via the different
 *      error responses (though in this case both would 500 for a wrong password).
 *
 * This PoC reproduces the crash using an in-memory MongoDB so it can be run standalone.
 *
 * Run: npx ts-node fuzz/poc/poc-002-null-deref.ts
 */

import { MongoMemoryServer } from 'mongodb-memory-server';
import mongoose, { Schema, Model, Document } from 'mongoose';

interface IUser extends Document {
  first_name: string;
  email: string;
  password: string;
  salt: string;
}

const UserSchema = new Schema<IUser>({
  first_name: { type: String, required: true },
  email:      { type: String, unique: true },
  password:   { type: String, required: true },
  salt:       { type: String },
});

// Replicates the EXACT vulnerable line from user.service.ts:16
async function vulnerableGetUser(
  usersModel: Model<IUser>,
  email: string
): Promise<IUser> {
  // BUG: no null check before .populate()
  return await (await usersModel.findOne({ email })).populate(['courses_enrolled', 'courses_created']);
}

// The fixed version for comparison
async function fixedGetUser(
  usersModel: Model<IUser>,
  email: string
): Promise<IUser | null> {
  const user = await usersModel.findOne({ email });
  if (!user) return null;   // ← the fix: return null rather than crash
  return user.populate(['courses_enrolled', 'courses_created']);
}

(async () => {
  console.log('PoC-002 — Null Dereference in UserService.getUser');
  console.log('SECURITY.md reference: NPE-001\n');

  const mongod = await MongoMemoryServer.create();
  const uri = mongod.getUri();
  await mongoose.connect(uri);

  const UserModel = mongoose.model<IUser>('Users', UserSchema);

  // --- Test 1: Non-existent user (the main exploit) ---
  console.log('Test 1: Call getUser() with an email that does not exist in the DB');
  console.log('        Reproduces the crash from auth.service.ts via login().\n');

  try {
    await vulnerableGetUser(UserModel, 'nonexistent@example.com');
    console.log('ERROR: Expected a TypeError but none was thrown — patch may be applied.');
    process.exit(1);
  } catch (err: unknown) {
    if (err instanceof TypeError && err.message.includes('populate')) {
      console.log('✓  CRASH REPRODUCED:');
      console.log(`   ${err.name}: ${err.message}`);
      console.log('   This is the error that reaches the NestJS 500 handler.\n');
    } else {
      console.log('UNEXPECTED error (not the expected TypeError):');
      console.log(err);
    }
  }

  // --- Test 2: Valid user (should not crash) ---
  console.log('Test 2: Call getUser() with an email that EXISTS — should not crash.');

  await UserModel.create({
    first_name: 'Test',
    email: 'existing@example.com',
    password: '$2b$10$fakehash',
    salt: '$2b$10$fakesalt',
  });

  try {
    const user = await vulnerableGetUser(UserModel, 'existing@example.com');
    console.log(`✓  No crash for existing user: ${user?.email}\n`);
  } catch (err) {
    console.log('Unexpected crash even for existing user:', err);
  }

  // --- Test 3: Fixed version (confirm fix works) ---
  console.log('Test 3: Fixed version — null check before .populate().');

  const result = await fixedGetUser(UserModel, 'nonexistent@example.com');
  if (result === null) {
    console.log('✓  Fixed version returns null instead of crashing.\n');
  }

  // --- Summary ---
  console.log('─'.repeat(60));
  console.log('SUMMARY');
  console.log('Vulnerable: (await findOne(email)).populate() — crashes if no user.');
  console.log('Fixed:      Check for null before calling .populate().');
  console.log('See SECURITY.md §NPE-001 for the corrected code.');

  await mongoose.disconnect();
  await mongod.stop();
})();
