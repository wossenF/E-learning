import { Controller, Get, Post, Body, Param, Query, UseGuards, Req } from '@nestjs/common';
import { MediaService } from './media.service';
import { UploadSubtitleDto } from './dtos/upload-subtitle.dto';
import { JwtAuthGuard } from '../auth/jwt.strategy';

@Controller('media')
export class MediaController {
  constructor(private readonly mediaService: MediaService) {}

  @Post('assets')
  @UseGuards(JwtAuthGuard)
  public async createMediaAsset(
    @Body('courseId') courseId: string,
    @Body('title') title: string,
    @Body('mediaUrl') mediaUrl: string,
    @Body('mediaType') mediaType: 'video' | 'audio' | 'document',
    @Body('durationSeconds') durationSeconds?: number,
  ) {
    return this.mediaService.createMediaAsset(courseId, title, mediaUrl, mediaType, durationSeconds || 0);
  }

  @Post('subtitles')
  @UseGuards(JwtAuthGuard)
  public async uploadSubtitles(@Body() dto: UploadSubtitleDto) {
    return this.mediaService.uploadSubtitleTrack(dto);
  }

  @Get(':id/transcript-search')
  public async searchTranscript(@Param('id') id: string, @Query('q') query: string) {
    return this.mediaService.searchTranscript(id, query);
  }

  @Get(':id/stream-token')
  @UseGuards(JwtAuthGuard)
  public async getStreamToken(@Req() req: any, @Param('id') id: string) {
    const userId = req.user.userId || req.user.id || req.user._id;
    return this.mediaService.getStreamToken(userId, id);
  }

  @Get('course/:courseId')
  public async getCourseMediaAssets(@Param('courseId') courseId: string) {
    return this.mediaService.getCourseMediaAssets(courseId);
  }
}
