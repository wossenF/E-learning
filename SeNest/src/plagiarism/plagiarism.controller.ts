import { Controller, Post, Body, UseGuards } from '@nestjs/common';
import { PlagiarismService } from './plagiarism.service';
import { CheckPlagiarismDto } from './dtos/check-plagiarism.dto';
import { JwtAuthGuard } from '../auth/jwt.strategy';

@Controller('plagiarism')
export class PlagiarismController {
  constructor(private readonly plagiarismService: PlagiarismService) {}

  @Post('analyze')
  @UseGuards(JwtAuthGuard)
  public analyzePlagiarism(@Body() dto: CheckPlagiarismDto) {
    return this.plagiarismService.analyzePlagiarism(dto);
  }

  @Post('diff')
  @UseGuards(JwtAuthGuard)
  public generateSubmissionDiff(@Body('sourceText') sourceText: string, @Body('targetText') targetText: string) {
    return this.plagiarismService.generateSubmissionDiff(sourceText || '', targetText || '');
  }
}
