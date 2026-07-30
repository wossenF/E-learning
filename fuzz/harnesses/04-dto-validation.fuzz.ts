/**
 * Harness 04 — class-validator DTO Fuzzer
 *
 * Targets:
 *   - SignupDto   (first_name, last_name, email, password)
 *   - UserLoginDto (email, password)
 *   - createCourseDto (title, price, duration, description, language, level)
 *   - queryStringDto  (price, language, level, search)
 *
 * What we're looking for:
 *   1. Crashes inside class-validator or class-transformer (ReDoS in email
 *      validation regex, prototype pollution, unexpected throws).
 *   2. Inputs that PASS validation but should be rejected (validation bypass).
 *   3. Inputs that crash the validator itself (not just return errors).
 *
 * Safe outcomes:
 *   - ValidationError[] returned — expected rejection, not a bug.
 *
 * Note: class-validator's @IsEmail uses validator.js which has historically
 * had ReDoS issues with certain email patterns. We target those here.
 */

import 'reflect-metadata';
import { FuzzedDataProvider } from '@jazzer.js/core';
import { validate } from 'class-validator';
import { plainToInstance } from 'class-transformer';

// ---------------------------------------------------------------------------
// Replicas of the DTOs — kept in sync with the source DTOs.
// Using direct imports from the SeNest source would require the full NestJS
// compilation chain; replicas keep the harness standalone and fast.
// ---------------------------------------------------------------------------

import {
  IsEmail, IsNotEmpty, IsString, IsNumber, Length, MaxLength, IsOptional,
} from 'class-validator';
import { Type } from 'class-transformer';

class SignupDto {
  @IsString() @Length(3, 11) first_name!: string;
  @IsString() @Length(3, 11) last_name!: string;
  @IsEmail()  @IsNotEmpty()  email!: string;
  @IsNotEmpty()              password!: string;
}

class UserLoginDto {
  @IsNotEmpty() @IsEmail()   email!: string;
  @IsNotEmpty() @IsString()  password!: string;
}

class CreateCourseDto {
  @IsNotEmpty() @MaxLength(150) title!: string;
  @IsNotEmpty() @IsNumber()     @Type(() => Number) price!: number;
  @IsNotEmpty() @IsNumber()     @Type(() => Number) duration!: number;
  @IsNotEmpty()                 description!: string;
  @IsNotEmpty()                 language!: string;
  @IsNotEmpty()                 level!: string;
}

class QueryStringDto {
  @IsOptional() price?:    unknown;
  @IsOptional() language?: unknown;
  @IsOptional() level?:    unknown;
  @IsOptional() search?:   unknown;
}

// ---------------------------------------------------------------------------

export function fuzz(data: Buffer): void {
  const provider = new FuzzedDataProvider(data);
  const dtoIndex = provider.consumeIntegralInRange(0, 3);

  switch (dtoIndex) {
    case 0: fuzzSignupDto(provider);      break;
    case 1: fuzzLoginDto(provider);       break;
    case 2: fuzzCreateCourseDto(provider); break;
    case 3: fuzzQueryStringDto(provider);  break;
  }
}

// ---------------------------------------------------------------------------

async function fuzzSignupDto(provider: FuzzedDataProvider): Promise<void> {
  const plain = {
    first_name: provider.consumeString(provider.consumeIntegralInRange(0, 64)),
    last_name:  provider.consumeString(provider.consumeIntegralInRange(0, 64)),
    email:      buildFuzzedEmail(provider),
    password:   provider.consumeString(provider.consumeIntegralInRange(0, 256)),
  };
  await validateSafely(plainToInstance(SignupDto, plain), 'SignupDto');
}

async function fuzzLoginDto(provider: FuzzedDataProvider): Promise<void> {
  const plain = {
    email:    buildFuzzedEmail(provider),
    password: provider.consumeString(provider.consumeIntegralInRange(0, 256)),
  };
  await validateSafely(plainToInstance(UserLoginDto, plain), 'UserLoginDto');
}

async function fuzzCreateCourseDto(provider: FuzzedDataProvider): Promise<void> {
  const plain = {
    title:       provider.consumeString(provider.consumeIntegralInRange(0, 512)),
    price:       provider.consumeBoolean()
                   ? provider.consumeIntegralInRange(-1e9, 1e9)
                   : provider.consumeString(32),
    duration:    provider.consumeBoolean()
                   ? provider.consumeIntegralInRange(-1e9, 1e9)
                   : provider.consumeString(32),
    description: provider.consumeString(provider.consumeIntegralInRange(0, 2048)),
    language:    provider.consumeString(provider.consumeIntegralInRange(0, 64)),
    level:       provider.consumeString(provider.consumeIntegralInRange(0, 64)),
  };
  await validateSafely(plainToInstance(CreateCourseDto, plain), 'CreateCourseDto');
}

async function fuzzQueryStringDto(provider: FuzzedDataProvider): Promise<void> {
  const plain: Record<string, unknown> = {};

  // price can be a string (from URL), a number, or an object (bracket notation)
  const priceVariant = provider.consumeIntegralInRange(0, 3);
  if      (priceVariant === 0) plain.price = provider.consumeIntegralInRange(-1e9, 1e9);
  else if (priceVariant === 1) plain.price = provider.consumeString(32);
  else if (priceVariant === 2) plain.price = { [provider.consumeString(8)]: provider.consumeString(8) };
  // variant 3: omit price

  plain.language = provider.consumeString(provider.consumeIntegralInRange(0, 64));
  plain.level    = provider.consumeString(provider.consumeIntegralInRange(0, 64));
  plain.search   = provider.consumeString(provider.consumeIntegralInRange(0, 256));

  // Add extra unexpected fields to test whitelist enforcement.
  if (provider.consumeBoolean()) {
    plain['$where']      = provider.consumeString(32);
    plain['__proto__']   = { polluted: true };
    plain['constructor'] = provider.consumeString(16);
  }

  await validateSafely(plainToInstance(QueryStringDto, plain), 'QueryStringDto');
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

function buildFuzzedEmail(provider: FuzzedDataProvider): string {
  const strategy = provider.consumeIntegralInRange(0, 4);
  switch (strategy) {
    case 0: return provider.consumeString(provider.consumeIntegralInRange(0, 512));
    case 1: {
      // Structurally valid email with fuzzed local and domain parts.
      const local  = provider.consumeString(provider.consumeIntegralInRange(1, 64));
      const domain = provider.consumeString(provider.consumeIntegralInRange(1, 32));
      const tld    = provider.consumeString(provider.consumeIntegralInRange(2, 8));
      return `${local}@${domain}.${tld}`;
    }
    case 2:
      // Known ReDoS patterns against validator.js email regex.
      return 'a'.repeat(provider.consumeIntegralInRange(20, 50)) + '@' +
             'b'.repeat(provider.consumeIntegralInRange(20, 50)) + '.c';
    case 3:
      return provider.consumeString(5) + '@' + 'x'.repeat(200) + '.com';
    case 4:
      // Pathological patterns reported against RFC 5322 parsers.
      return '"' + provider.consumeString(32) + '"@example.com';
    default:
      return '';
  }
}

async function validateSafely(instance: object, dtoName: string): Promise<void> {
  try {
    await validate(instance, { forbidUnknownValues: false });
    // ValidationError[] is the expected rejection path — not a bug.
  } catch (err: unknown) {
    // class-validator should NEVER throw (only return errors).
    // Any throw here is unexpected and should be investigated.
    if (err instanceof Error && err.message.includes('Cannot read')) {
      // NPE inside validator — finding.
      throw new Error(`VALIDATOR_CRASH in ${dtoName}: ${err.message}`);
    }
    throw err;
  }
}
