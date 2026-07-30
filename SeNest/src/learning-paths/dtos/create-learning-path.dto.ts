import {
  ArrayMaxSize,
  ArrayMinSize,
  IsArray,
  IsBoolean,
  IsDateString,
  IsEnum,
  IsInt,
  IsMongoId,
  IsNotEmpty,
  IsOptional,
  IsString,
  Length,
  Max,
  Min,
  ValidateNested,
} from 'class-validator';
import { Type } from 'class-transformer';

class MilestoneProjectDto {
  @IsString()
  @Length(5, 120)
  title: string;

  @IsString()
  @Length(20, 600)
  brief: string;

  @IsArray()
  @IsString({ each: true })
  @ArrayMaxSize(20)
  acceptanceCriteria: string[];

  @IsArray()
  @IsString({ each: true })
  @ArrayMaxSize(20)
  deliverables: string[];

  @IsArray()
  @IsString({ each: true })
  @ArrayMaxSize(20)
  tags: string[];

  @IsInt()
  @Min(10)
  @Max(600)
  estimatedMinutes: number;
}

class PathMilestoneDto {
  @IsString()
  @Length(2, 40)
  id: string;

  @IsString()
  @Length(5, 120)
  title: string;

  @IsString()
  @Length(20, 600)
  objective: string;

  @IsArray()
  @IsString({ each: true })
  @ArrayMaxSize(100)
  requiredCourseIds: string[];

  @IsArray()
  @IsString({ each: true })
  @ArrayMaxSize(100)
  optionalCourseIds: string[];

  @IsArray()
  @IsString({ each: true })
  @ArrayMaxSize(100)
  prerequisiteMilestoneIds: string[];

  @ValidateNested({ each: true })
  @Type(() => MilestoneProjectDto)
  @IsArray()
  @ArrayMaxSize(20)
  projects: MilestoneProjectDto[];

  @IsInt()
  @Min(15)
  @Max(2400)
  estimatedMinutes: number;

  @IsInt()
  @Min(0)
  @Max(100)
  unlockWeight: number;

  @IsArray()
  @IsString({ each: true })
  @ArrayMaxSize(25)
  rubric: string[];

  @IsArray()
  @IsString({ each: true })
  @ArrayMaxSize(25)
  completionSignals: string[];
}

class CohortPlanDto {
  @IsString()
  @Length(2, 60)
  cohortId: string;

  @IsInt()
  @Min(1)
  @Max(200)
  seatCount: number;

  @IsDateString()
  startsAt: string;

  @IsDateString()
  endsAt: string;

  @IsArray()
  @IsString({ each: true })
  coachIds: string[];

  @IsArray()
  @IsString({ each: true })
  timezoneWindows: string[];

  @IsArray()
  @IsString({ each: true })
  communicationChannels: string[];
}

class CompletionPolicyDto {
  @IsInt()
  @Min(0)
  @Max(100)
  minQuizScore: number;

  @IsInt()
  @Min(0)
  @Max(100)
  minProjectRubricScore: number;

  @IsInt()
  @Min(0)
  @Max(100)
  minAttendance: number;

  @IsArray()
  @IsString({ each: true })
  requiredMilestoneIds: string[];

  @IsInt()
  @Min(1)
  @Max(30)
  maxInactivityDays: number;

  @IsBoolean()
  requiresCoachSignoff: boolean;
}

export class CreateLearningPathDto {
  @IsString()
  @Length(8, 160)
  title: string;

  @IsString()
  @Length(30, 1200)
  summary: string;

  @IsString()
  @Length(50, 4000)
  longDescription: string;

  @IsString()
  @Length(3, 120)
  slug: string;

  @IsArray()
  @IsString({ each: true })
  @ArrayMaxSize(40)
  tags: string[];

  @IsArray()
  @IsString({ each: true })
  @ArrayMaxSize(40)
  competencies: string[];

  @IsArray()
  @IsString({ each: true })
  @ArrayMaxSize(40)
  tools: string[];

  @IsArray()
  @IsString({ each: true })
  @ArrayMaxSize(40)
  audience: string[];

  @ValidateNested({ each: true })
  @Type(() => PathMilestoneDto)
  @IsArray()
  @ArrayMinSize(1)
  @ArrayMaxSize(100)
  milestones: PathMilestoneDto[];

  @ValidateNested({ each: true })
  @Type(() => CohortPlanDto)
  @IsOptional()
  @IsArray()
  @ArrayMaxSize(60)
  cohortPlans?: CohortPlanDto[];

  @ValidateNested()
  @Type(() => CompletionPolicyDto)
  completionPolicy: CompletionPolicyDto;

  @IsInt()
  @Min(1)
  @Max(52)
  suggestedDurationWeeks: number;

  @IsInt()
  @Min(1)
  @Max(2000)
  expectedEffortHours: number;

  @IsInt()
  @Min(0)
  @Max(100000)
  priceUsdCents: number;

  @IsEnum(['draft', 'review', 'published', 'archived'])
  status: 'draft' | 'review' | 'published' | 'archived';

  @IsBoolean()
  featured: boolean;

  @IsArray()
  @IsString({ each: true })
  partnerOrganizations: string[];

  @IsArray()
  @IsString({ each: true })
  localeAvailability: string[];

  @IsArray()
  @IsString({ each: true })
  prerequisitesText: string[];

  @IsArray()
  @IsString({ each: true })
  outcomes: string[];

  @IsArray()
  @IsString({ each: true })
  capstoneRequirements: string[];

  @IsInt()
  @Min(0)
  @Max(100)
  qualityScore: number;

  @IsInt()
  @Min(0)
  @Max(100)
  discoverabilityScore: number;

  @IsMongoId()
  @IsNotEmpty()
  ownerId: string;

  @IsArray()
  @IsMongoId({ each: true })
  @IsOptional()
  maintainerIds?: string[];

  @IsArray()
  @IsMongoId({ each: true })
  @IsOptional()
  relatedCourseIds?: string[];
}
