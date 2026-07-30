import {
  IsArray,
  IsBoolean,
  IsEnum,
  IsInt,
  IsMongoId,
  IsOptional,
  IsString,
  Length,
  Max,
  Min,
  ValidateNested,
} from 'class-validator';
import { Type } from 'class-transformer';

class UpdateMilestoneProjectDto {
  @IsOptional()
  @IsString()
  @Length(5, 120)
  title?: string;

  @IsOptional()
  @IsString()
  @Length(20, 600)
  brief?: string;

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  acceptanceCriteria?: string[];

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  deliverables?: string[];

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  tags?: string[];

  @IsOptional()
  @IsInt()
  @Min(10)
  @Max(600)
  estimatedMinutes?: number;
}

class UpdatePathMilestoneDto {
  @IsOptional()
  @IsString()
  @Length(2, 40)
  id?: string;

  @IsOptional()
  @IsString()
  @Length(5, 120)
  title?: string;

  @IsOptional()
  @IsString()
  @Length(20, 600)
  objective?: string;

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  requiredCourseIds?: string[];

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  optionalCourseIds?: string[];

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  prerequisiteMilestoneIds?: string[];

  @IsOptional()
  @ValidateNested({ each: true })
  @Type(() => UpdateMilestoneProjectDto)
  @IsArray()
  projects?: UpdateMilestoneProjectDto[];

  @IsOptional()
  @IsInt()
  @Min(15)
  @Max(2400)
  estimatedMinutes?: number;

  @IsOptional()
  @IsInt()
  @Min(0)
  @Max(100)
  unlockWeight?: number;

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  rubric?: string[];

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  completionSignals?: string[];
}

class UpdateCompletionPolicyDto {
  @IsOptional()
  @IsInt()
  @Min(0)
  @Max(100)
  minQuizScore?: number;

  @IsOptional()
  @IsInt()
  @Min(0)
  @Max(100)
  minProjectRubricScore?: number;

  @IsOptional()
  @IsInt()
  @Min(0)
  @Max(100)
  minAttendance?: number;

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  requiredMilestoneIds?: string[];

  @IsOptional()
  @IsInt()
  @Min(1)
  @Max(30)
  maxInactivityDays?: number;

  @IsOptional()
  @IsBoolean()
  requiresCoachSignoff?: boolean;
}

export class UpdateLearningPathDto {
  @IsOptional()
  @IsString()
  @Length(8, 160)
  title?: string;

  @IsOptional()
  @IsString()
  @Length(30, 1200)
  summary?: string;

  @IsOptional()
  @IsString()
  @Length(50, 4000)
  longDescription?: string;

  @IsOptional()
  @IsString()
  @Length(3, 120)
  slug?: string;

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  tags?: string[];

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  competencies?: string[];

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  tools?: string[];

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  audience?: string[];

  @IsOptional()
  @ValidateNested({ each: true })
  @Type(() => UpdatePathMilestoneDto)
  @IsArray()
  milestones?: UpdatePathMilestoneDto[];

  @IsOptional()
  @ValidateNested()
  @Type(() => UpdateCompletionPolicyDto)
  completionPolicy?: UpdateCompletionPolicyDto;

  @IsOptional()
  @IsInt()
  @Min(1)
  @Max(52)
  suggestedDurationWeeks?: number;

  @IsOptional()
  @IsInt()
  @Min(1)
  @Max(2000)
  expectedEffortHours?: number;

  @IsOptional()
  @IsInt()
  @Min(0)
  @Max(100000)
  priceUsdCents?: number;

  @IsOptional()
  @IsEnum(['draft', 'review', 'published', 'archived'])
  status?: 'draft' | 'review' | 'published' | 'archived';

  @IsOptional()
  @IsBoolean()
  featured?: boolean;

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  partnerOrganizations?: string[];

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  localeAvailability?: string[];

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  prerequisitesText?: string[];

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  outcomes?: string[];

  @IsOptional()
  @IsArray()
  @IsString({ each: true })
  capstoneRequirements?: string[];

  @IsOptional()
  @IsInt()
  @Min(0)
  @Max(100)
  qualityScore?: number;

  @IsOptional()
  @IsInt()
  @Min(0)
  @Max(100)
  discoverabilityScore?: number;

  @IsOptional()
  @IsMongoId()
  ownerId?: string;

  @IsOptional()
  @IsArray()
  @IsMongoId({ each: true })
  maintainerIds?: string[];

  @IsOptional()
  @IsArray()
  @IsMongoId({ each: true })
  relatedCourseIds?: string[];
}
