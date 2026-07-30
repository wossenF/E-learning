import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';
import { HydratedDocument, Types } from 'mongoose';

export type LearningPathDocument = HydratedDocument<LearningPath>;

@Schema({ _id: false })
export class MilestoneProject {
  @Prop({ required: true, trim: true })
  title: string;

  @Prop({ required: true, trim: true })
  brief: string;

  @Prop({ type: [String], default: [] })
  acceptanceCriteria: string[];

  @Prop({ type: [String], default: [] })
  deliverables: string[];

  @Prop({ type: [String], default: [] })
  tags: string[];

  @Prop({ min: 10, max: 600, default: 90 })
  estimatedMinutes: number;
}

@Schema({ _id: false })
export class PathMilestone {
  @Prop({ required: true, trim: true })
  id: string;

  @Prop({ required: true, trim: true })
  title: string;

  @Prop({ required: true, trim: true })
  objective: string;

  @Prop({ type: [String], default: [] })
  requiredCourseIds: string[];

  @Prop({ type: [String], default: [] })
  optionalCourseIds: string[];

  @Prop({ type: [String], default: [] })
  prerequisiteMilestoneIds: string[];

  @Prop({ type: [MilestoneProject], default: [] })
  projects: MilestoneProject[];

  @Prop({ min: 15, max: 2400, default: 240 })
  estimatedMinutes: number;

  @Prop({ min: 0, max: 100, default: 1 })
  unlockWeight: number;

  @Prop({ type: [String], default: [] })
  rubric: string[];

  @Prop({ type: [String], default: [] })
  completionSignals: string[];
}

@Schema({ _id: false })
export class CohortPlan {
  @Prop({ required: true, trim: true })
  cohortId: string;

  @Prop({ min: 1, max: 200, default: 30 })
  seatCount: number;

  @Prop({ required: true })
  startsAt: Date;

  @Prop({ required: true })
  endsAt: Date;

  @Prop({ type: [String], default: [] })
  coachIds: string[];

  @Prop({ type: [String], default: [] })
  timezoneWindows: string[];

  @Prop({ type: [String], default: [] })
  communicationChannels: string[];
}

@Schema({ _id: false })
export class CompletionPolicy {
  @Prop({ min: 0, max: 100, default: 70 })
  minQuizScore: number;

  @Prop({ min: 0, max: 100, default: 75 })
  minProjectRubricScore: number;

  @Prop({ min: 0, max: 100, default: 80 })
  minAttendance: number;

  @Prop({ type: [String], default: [] })
  requiredMilestoneIds: string[];

  @Prop({ min: 1, max: 30, default: 7 })
  maxInactivityDays: number;

  @Prop({ default: true })
  requiresCoachSignoff: boolean;
}

@Schema({ _id: false })
export class LearningPathStats {
  @Prop({ min: 0, default: 0 })
  enrollmentCount: number;

  @Prop({ min: 0, max: 100, default: 0 })
  completionRate: number;

  @Prop({ min: 0, default: 0 })
  averageCompletionDays: number;

  @Prop({ min: 0, max: 100, default: 0 })
  averageSatisfaction: number;

  @Prop({ min: 0, default: 0 })
  dropoffAfterMilestoneIndex: number;

  @Prop({ type: [Number], default: [] })
  weeklyActiveLearners: number[];
}

@Schema({ timestamps: true })
export class LearningPath {
  @Prop({ required: true, trim: true, minlength: 8, maxlength: 160 })
  title: string;

  @Prop({ required: true, trim: true, minlength: 30, maxlength: 1200 })
  summary: string;

  @Prop({ required: true, trim: true, minlength: 50, maxlength: 4000 })
  longDescription: string;

  @Prop({ required: true, trim: true })
  slug: string;

  @Prop({ type: [String], default: [] })
  tags: string[];

  @Prop({ type: [String], default: [] })
  competencies: string[];

  @Prop({ type: [String], default: [] })
  tools: string[];

  @Prop({ type: [String], default: [] })
  audience: string[];

  @Prop({ type: [PathMilestone], default: [] })
  milestones: PathMilestone[];

  @Prop({ type: [CohortPlan], default: [] })
  cohortPlans: CohortPlan[];

  @Prop({ type: CompletionPolicy, default: () => ({}) })
  completionPolicy: CompletionPolicy;

  @Prop({ type: LearningPathStats, default: () => ({}) })
  stats: LearningPathStats;

  @Prop({ min: 1, max: 52, default: 8 })
  suggestedDurationWeeks: number;

  @Prop({ min: 1, max: 2000, default: 120 })
  expectedEffortHours: number;

  @Prop({ min: 0, max: 100000, default: 0 })
  priceUsdCents: number;

  @Prop({ type: String, enum: ['draft', 'review', 'published', 'archived'], default: 'draft' })
  status: 'draft' | 'review' | 'published' | 'archived';

  @Prop({ default: false })
  featured: boolean;

  @Prop({ type: [String], default: [] })
  partnerOrganizations: string[];

  @Prop({ type: [String], default: [] })
  localeAvailability: string[];

  @Prop({ type: [String], default: [] })
  prerequisitesText: string[];

  @Prop({ type: [String], default: [] })
  outcomes: string[];

  @Prop({ type: [String], default: [] })
  capstoneRequirements: string[];

  @Prop({ min: 0, max: 100, default: 0 })
  qualityScore: number;

  @Prop({ min: 0, max: 100, default: 0 })
  discoverabilityScore: number;

  @Prop({ type: Types.ObjectId, ref: 'Users', required: true })
  ownerId: Types.ObjectId;

  @Prop({ type: [Types.ObjectId], ref: 'Users', default: [] })
  maintainerIds: Types.ObjectId[];

  @Prop({ type: [Types.ObjectId], ref: 'Courses', default: [] })
  relatedCourseIds: Types.ObjectId[];

  @Prop({ type: Date, default: null })
  publishedAt: Date;

  @Prop({ type: Date, default: null })
  archivedAt: Date;
}

export const LearningPathSchema = SchemaFactory.createForClass(LearningPath);

LearningPathSchema.index({ slug: 1 }, { unique: true });
LearningPathSchema.index({ status: 1, featured: 1, qualityScore: -1 });
LearningPathSchema.index({ tags: 1, competencies: 1 });
LearningPathSchema.index({ ownerId: 1, status: 1 });
