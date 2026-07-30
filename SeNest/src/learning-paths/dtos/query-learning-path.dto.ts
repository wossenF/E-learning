import { Transform } from 'class-transformer';
import { IsBoolean, IsEnum, IsInt, IsOptional, IsString, Max, Min } from 'class-validator';

function parseBoolean(value: unknown): boolean | undefined {
  if (value === undefined || value === null || value === '') {
    return undefined;
  }
  if (typeof value === 'boolean') {
    return value;
  }
  const normalized = String(value).toLowerCase().trim();
  if (['true', '1', 'yes', 'y'].includes(normalized)) {
    return true;
  }
  if (['false', '0', 'no', 'n'].includes(normalized)) {
    return false;
  }
  return undefined;
}

function parseInteger(value: unknown): number | undefined {
  if (value === undefined || value === null || value === '') {
    return undefined;
  }
  const parsed = Number(value);
  if (!Number.isFinite(parsed)) {
    return undefined;
  }
  return Math.trunc(parsed);
}

function parseStringArray(value: unknown): string[] {
  if (value === undefined || value === null || value === '') {
    return [];
  }
  if (Array.isArray(value)) {
    return value
      .map((item) => String(item).trim())
      .filter((item) => item.length > 0);
  }
  return String(value)
    .split(',')
    .map((item) => item.trim())
    .filter((item) => item.length > 0);
}

export class QueryLearningPathDto {
  @IsOptional()
  @IsString()
  search?: string;

  @IsOptional()
  @Transform(({ value }) => parseStringArray(value))
  tags?: string[];

  @IsOptional()
  @Transform(({ value }) => parseStringArray(value))
  competencies?: string[];

  @IsOptional()
  @Transform(({ value }) => parseStringArray(value))
  audience?: string[];

  @IsOptional()
  @Transform(({ value }) => parseInteger(value))
  @IsInt()
  @Min(0)
  @Max(100000)
  minPriceUsdCents?: number;

  @IsOptional()
  @Transform(({ value }) => parseInteger(value))
  @IsInt()
  @Min(0)
  @Max(100000)
  maxPriceUsdCents?: number;

  @IsOptional()
  @Transform(({ value }) => parseInteger(value))
  @IsInt()
  @Min(1)
  @Max(52)
  minDurationWeeks?: number;

  @IsOptional()
  @Transform(({ value }) => parseInteger(value))
  @IsInt()
  @Min(1)
  @Max(52)
  maxDurationWeeks?: number;

  @IsOptional()
  @Transform(({ value }) => parseInteger(value))
  @IsInt()
  @Min(1)
  @Max(2000)
  minEffortHours?: number;

  @IsOptional()
  @Transform(({ value }) => parseInteger(value))
  @IsInt()
  @Min(1)
  @Max(2000)
  maxEffortHours?: number;

  @IsOptional()
  @Transform(({ value }) => parseBoolean(value))
  @IsBoolean()
  featured?: boolean;

  @IsOptional()
  @IsEnum(['draft', 'review', 'published', 'archived'])
  status?: 'draft' | 'review' | 'published' | 'archived';

  @IsOptional()
  @Transform(({ value }) => parseInteger(value))
  @IsInt()
  @Min(1)
  @Max(200)
  limit?: number;

  @IsOptional()
  @Transform(({ value }) => parseInteger(value))
  @IsInt()
  @Min(1)
  page?: number;

  @IsOptional()
  @IsEnum([
    'relevance',
    'quality',
    'price-asc',
    'price-desc',
    'duration-asc',
    'duration-desc',
    'recent',
  ])
  sort?: 'relevance' | 'quality' | 'price-asc' | 'price-desc' | 'duration-asc' | 'duration-desc' | 'recent';
}
