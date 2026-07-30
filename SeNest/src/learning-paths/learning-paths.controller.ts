import {
  Body,
  Controller,
  Delete,
  Get,
  Param,
  Patch,
  Post,
  Query,
  UsePipes,
  ValidationPipe,
} from '@nestjs/common';
import { LearningPathsService } from './learning-paths.service';
import { CreateLearningPathDto } from './dtos/create-learning-path.dto';
import { UpdateLearningPathDto } from './dtos/update-learning-path.dto';
import { QueryLearningPathDto } from './dtos/query-learning-path.dto';

@Controller('learning-paths')
@UsePipes(new ValidationPipe({ transform: true, whitelist: true }))
export class LearningPathsController {
  constructor(private readonly learningPathsService: LearningPathsService) {}

  @Post()
  async create(@Body() payload: CreateLearningPathDto) {
    return await this.learningPathsService.create(payload);
  }

  @Get()
  async findAll(@Query() query: QueryLearningPathDto) {
    return await this.learningPathsService.findAll(query);
  }

  @Get(':id')
  async findById(@Param('id') id: string) {
    return await this.learningPathsService.findById(id);
  }

  @Patch(':id')
  async update(@Param('id') id: string, @Body() payload: UpdateLearningPathDto) {
    return await this.learningPathsService.update(id, payload);
  }

  @Delete(':id')
  async remove(@Param('id') id: string) {
    await this.learningPathsService.remove(id);
    return {
      ok: true,
      id,
    };
  }

  @Post(':id/recalculate-scores')
  async recalculateScores(@Param('id') id: string) {
    return await this.learningPathsService.recalculateScores(id);
  }

  @Get(':id/health')
  async healthReport(@Param('id') id: string) {
    return await this.learningPathsService.analyzeMilestoneHealth(id);
  }

  @Post(':id/clone')
  async clone(
    @Param('id') id: string,
    @Body() body: { ownerId: string; suffix?: string },
  ) {
    return await this.learningPathsService.clone(id, body.ownerId, body.suffix ?? 'copy');
  }

  @Patch(':id/stats')
  async updateStats(
    @Param('id') id: string,
    @Body()
    stats: {
      enrollmentCount?: number;
      completionRate?: number;
      averageCompletionDays?: number;
      averageSatisfaction?: number;
      dropoffAfterMilestoneIndex?: number;
      weeklyActiveLearners?: number[];
    },
  ) {
    return await this.learningPathsService.updateStats(id, stats);
  }
}
