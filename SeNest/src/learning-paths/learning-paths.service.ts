import {
  BadRequestException,
  Injectable,
  NotFoundException,
} from '@nestjs/common';
import { InjectModel } from '@nestjs/mongoose';
import { FilterQuery, Model, Types } from 'mongoose';
import {
  LearningPath,
  LearningPathDocument,
} from './Schemas/learning-path.schema';
import { CreateLearningPathDto } from './dtos/create-learning-path.dto';
import { UpdateLearningPathDto } from './dtos/update-learning-path.dto';
import { QueryLearningPathDto } from './dtos/query-learning-path.dto';
import {
  buildPathHealthReport,
  calculateDiscoverabilityScore,
  calculateQualityScore,
  ensureMilestoneGraphIsValid,
} from './utils/path-analysis.utils';

type ListResponse = {
  items: LearningPath[];
  count: number;
  page: number;
  limit: number;
};

@Injectable()
export class LearningPathsService {
  constructor(
    @InjectModel('LearningPath')
    private readonly learningPathModel: Model<LearningPathDocument>,
  ) {}

  async create(payload: CreateLearningPathDto): Promise<LearningPath> {
    ensureMilestoneGraphIsValid(payload.milestones);

    await this.ensureSlugIsAvailable(payload.slug);

    const computedScores = this.computeScoresFromPayload(payload);

    const doc = await this.learningPathModel.create({
      ...payload,
      qualityScore: computedScores.qualityScore,
      discoverabilityScore: computedScores.discoverabilityScore,
      ownerId: new Types.ObjectId(payload.ownerId),
      maintainerIds: (payload.maintainerIds ?? []).map((id) => new Types.ObjectId(id)),
      relatedCourseIds: (payload.relatedCourseIds ?? []).map((id) => new Types.ObjectId(id)),
      publishedAt: payload.status === 'published' ? new Date() : null,
      archivedAt: payload.status === 'archived' ? new Date() : null,
    });

    return await this.findById(doc.id);
  }

  async findAll(query: QueryLearningPathDto): Promise<ListResponse> {
    const page = Math.max(1, query.page ?? 1);
    const limit = Math.max(1, Math.min(200, query.limit ?? 20));

    const filter = this.buildFilter(query);
    const sort = this.buildSort(query.sort ?? 'relevance');

    const [items, count] = await Promise.all([
      this.learningPathModel
        .find(filter)
        .sort(sort)
        .skip((page - 1) * limit)
        .limit(limit)
        .lean(),
      this.learningPathModel.countDocuments(filter),
    ]);

    return {
      items: items as unknown as LearningPath[],
      count,
      page,
      limit,
    };
  }

  async findById(id: string): Promise<LearningPath> {
    if (!Types.ObjectId.isValid(id)) {
      throw new BadRequestException('Invalid learning path id.');
    }

    const doc = await this.learningPathModel.findById(id).lean();
    if (!doc) {
      throw new NotFoundException('Learning path not found.');
    }

    return doc as unknown as LearningPath;
  }

  async update(id: string, payload: UpdateLearningPathDto): Promise<LearningPath> {
    const current = await this.findById(id);

    const nextValue: any = {
      ...current,
      ...payload,
    };

    if (payload.ownerId) {
      nextValue.ownerId = new Types.ObjectId(payload.ownerId);
    }

    if (payload.maintainerIds) {
      nextValue.maintainerIds = payload.maintainerIds.map((value) => new Types.ObjectId(value));
    }

    if (payload.relatedCourseIds) {
      nextValue.relatedCourseIds = payload.relatedCourseIds.map((value) => new Types.ObjectId(value));
    }

    if (payload.slug && payload.slug !== current.slug) {
      await this.ensureSlugIsAvailable(payload.slug, id);
    }

    if (payload.milestones) {
      ensureMilestoneGraphIsValid(payload.milestones as any[]);
    }

    nextValue.qualityScore = calculateQualityScore({
      milestones: nextValue.milestones,
      completionPolicy: nextValue.completionPolicy,
      outcomes: nextValue.outcomes ?? [],
      capstoneRequirements: nextValue.capstoneRequirements ?? [],
    });

    nextValue.discoverabilityScore = calculateDiscoverabilityScore({
      title: nextValue.title ?? current.title,
      summary: nextValue.summary ?? current.summary,
      tags: nextValue.tags ?? [],
      competencies: nextValue.competencies ?? [],
      outcomes: nextValue.outcomes ?? [],
      featured: nextValue.featured ?? false,
      localeAvailability: nextValue.localeAvailability ?? [],
    });

    if (payload.status === 'published' && !current.publishedAt) {
      nextValue.publishedAt = new Date();
      nextValue.archivedAt = null;
    }

    if (payload.status === 'archived') {
      nextValue.archivedAt = new Date();
    }

    if (payload.status && payload.status !== 'archived' && current.archivedAt) {
      nextValue.archivedAt = null;
    }

    await this.learningPathModel.findByIdAndUpdate(id, nextValue, {
      returnDocument: 'after',
    });

    return await this.findById(id);
  }

  async remove(id: string): Promise<void> {
    if (!Types.ObjectId.isValid(id)) {
      throw new BadRequestException('Invalid learning path id.');
    }

    const result = await this.learningPathModel.findByIdAndDelete(id);
    if (!result) {
      throw new NotFoundException('Learning path not found.');
    }
  }

  async recalculateScores(id: string): Promise<LearningPath> {
    const current = await this.findById(id);

    const qualityScore = calculateQualityScore({
      milestones: current.milestones,
      completionPolicy: current.completionPolicy,
      outcomes: current.outcomes,
      capstoneRequirements: current.capstoneRequirements,
    });

    const discoverabilityScore = calculateDiscoverabilityScore({
      title: current.title,
      summary: current.summary,
      tags: current.tags,
      competencies: current.competencies,
      outcomes: current.outcomes,
      featured: current.featured,
      localeAvailability: current.localeAvailability,
    });

    await this.learningPathModel.findByIdAndUpdate(
      id,
      {
        qualityScore,
        discoverabilityScore,
      },
      { returnDocument: 'after' },
    );

    return await this.findById(id);
  }

  async analyzeMilestoneHealth(id: string) {
    const current = await this.findById(id);
    return buildPathHealthReport(current.milestones as any[]);
  }

  async clone(id: string, ownerId: string, suffix = 'copy'): Promise<LearningPath> {
    const source = await this.findById(id);

    const timestamp = new Date().toISOString().slice(0, 10).replace(/-/g, '');
    const clonedSlug = `${source.slug}-${suffix}-${timestamp}`;

    await this.ensureSlugIsAvailable(clonedSlug);

    const cloned = {
      ...source,
      title: `${source.title} (${suffix})`,
      slug: clonedSlug,
      status: 'draft' as const,
      ownerId,
      maintainerIds: [ownerId],
      publishedAt: null,
      archivedAt: null,
      stats: {
        enrollmentCount: 0,
        completionRate: 0,
        averageCompletionDays: 0,
        averageSatisfaction: 0,
        dropoffAfterMilestoneIndex: 0,
        weeklyActiveLearners: [],
      },
    };

    delete (cloned as any)._id;
    delete (cloned as any).id;
    delete (cloned as any).createdAt;
    delete (cloned as any).updatedAt;

    return await this.create(cloned as unknown as CreateLearningPathDto);
  }

  async updateStats(
    id: string,
    stats: {
      enrollmentCount?: number;
      completionRate?: number;
      averageCompletionDays?: number;
      averageSatisfaction?: number;
      dropoffAfterMilestoneIndex?: number;
      weeklyActiveLearners?: number[];
    },
  ): Promise<LearningPath> {
    const current = await this.findById(id);

    const nextStats = {
      enrollmentCount: this.clamp(stats.enrollmentCount ?? current.stats.enrollmentCount, 0, 1000000),
      completionRate: this.clamp(stats.completionRate ?? current.stats.completionRate, 0, 100),
      averageCompletionDays: this.clamp(
        stats.averageCompletionDays ?? current.stats.averageCompletionDays,
        0,
        3650,
      ),
      averageSatisfaction: this.clamp(
        stats.averageSatisfaction ?? current.stats.averageSatisfaction,
        0,
        100,
      ),
      dropoffAfterMilestoneIndex: this.clamp(
        stats.dropoffAfterMilestoneIndex ?? current.stats.dropoffAfterMilestoneIndex,
        0,
        Math.max(0, current.milestones.length - 1),
      ),
      weeklyActiveLearners: this.normalizeTrend(
        stats.weeklyActiveLearners ?? current.stats.weeklyActiveLearners,
      ),
    };

    await this.learningPathModel.findByIdAndUpdate(
      id,
      {
        stats: nextStats,
      },
      {
        returnDocument: 'after',
      },
    );

    return await this.findById(id);
  }

  private buildFilter(query: QueryLearningPathDto): FilterQuery<LearningPathDocument> {
    const filter: FilterQuery<LearningPathDocument> = {};

    if (query.status) {
      filter.status = query.status;
    }

    if (query.featured !== undefined) {
      filter.featured = query.featured;
    }

    if (query.search && query.search.trim().length > 0) {
      const regex = { $regex: query.search.trim(), $options: 'i' };
      filter.$or = [
        { title: regex },
        { summary: regex },
        { longDescription: regex },
        { tags: regex },
        { competencies: regex },
      ];
    }

    if ((query.tags ?? []).length > 0) {
      filter.tags = { $in: query.tags };
    }

    if ((query.competencies ?? []).length > 0) {
      filter.competencies = { $in: query.competencies };
    }

    if ((query.audience ?? []).length > 0) {
      filter.audience = { $in: query.audience };
    }

    if (query.minPriceUsdCents !== undefined || query.maxPriceUsdCents !== undefined) {
      filter.priceUsdCents = {};
      if (query.minPriceUsdCents !== undefined) {
        (filter.priceUsdCents as any).$gte = query.minPriceUsdCents;
      }
      if (query.maxPriceUsdCents !== undefined) {
        (filter.priceUsdCents as any).$lte = query.maxPriceUsdCents;
      }
    }

    if (query.minDurationWeeks !== undefined || query.maxDurationWeeks !== undefined) {
      filter.suggestedDurationWeeks = {};
      if (query.minDurationWeeks !== undefined) {
        (filter.suggestedDurationWeeks as any).$gte = query.minDurationWeeks;
      }
      if (query.maxDurationWeeks !== undefined) {
        (filter.suggestedDurationWeeks as any).$lte = query.maxDurationWeeks;
      }
    }

    if (query.minEffortHours !== undefined || query.maxEffortHours !== undefined) {
      filter.expectedEffortHours = {};
      if (query.minEffortHours !== undefined) {
        (filter.expectedEffortHours as any).$gte = query.minEffortHours;
      }
      if (query.maxEffortHours !== undefined) {
        (filter.expectedEffortHours as any).$lte = query.maxEffortHours;
      }
    }

    return filter;
  }

  private buildSort(sort: QueryLearningPathDto['sort']): Record<string, 1 | -1> {
    switch (sort) {
      case 'quality':
        return { qualityScore: -1, discoverabilityScore: -1, updatedAt: -1 };
      case 'price-asc':
        return { priceUsdCents: 1, qualityScore: -1 };
      case 'price-desc':
        return { priceUsdCents: -1, qualityScore: -1 };
      case 'duration-asc':
        return { suggestedDurationWeeks: 1, qualityScore: -1 };
      case 'duration-desc':
        return { suggestedDurationWeeks: -1, qualityScore: -1 };
      case 'recent':
        return { updatedAt: -1 };
      case 'relevance':
      default:
        return { featured: -1, discoverabilityScore: -1, qualityScore: -1, updatedAt: -1 };
    }
  }

  private computeScoresFromPayload(payload: CreateLearningPathDto): {
    qualityScore: number;
    discoverabilityScore: number;
  } {
    const qualityScore = calculateQualityScore({
      milestones: payload.milestones,
      completionPolicy: payload.completionPolicy,
      outcomes: payload.outcomes,
      capstoneRequirements: payload.capstoneRequirements,
    });

    const discoverabilityScore = calculateDiscoverabilityScore({
      title: payload.title,
      summary: payload.summary,
      tags: payload.tags,
      competencies: payload.competencies,
      outcomes: payload.outcomes,
      featured: payload.featured,
      localeAvailability: payload.localeAvailability,
    });

    return {
      qualityScore,
      discoverabilityScore,
    };
  }

  private async ensureSlugIsAvailable(slug: string, ignoreId?: string): Promise<void> {
    const existing = await this.learningPathModel.findOne({ slug }).lean();
    if (!existing) {
      return;
    }

    if (ignoreId && String(existing._id) === ignoreId) {
      return;
    }

    throw new BadRequestException(`Learning path slug ${slug} is already in use.`);
  }

  private clamp(value: number, min: number, max: number): number {
    return Math.max(min, Math.min(max, value));
  }

  private normalizeTrend(values: number[]): number[] {
    if (!Array.isArray(values)) {
      return [];
    }
    const trimmed = values.slice(-52);
    return trimmed.map((value) => this.clamp(Math.trunc(value), 0, 1000000));
  }
}
