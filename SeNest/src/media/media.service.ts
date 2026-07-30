import { Injectable, NotFoundException, BadRequestException, ForbiddenException } from '@nestjs/common';
import { InjectModel } from '@nestjs/mongoose';
import { Model, Types } from 'mongoose';
import { MediaAsset, MediaAssetDocument } from './schemas/media-asset.schema';
import { UploadSubtitleDto } from './dtos/upload-subtitle.dto';
import { SrtVttParserUtils, SubtitleCue } from './utils/srt-vtt-parser.utils';
import { MediaTokenUtils } from './utils/media-token.utils';

export interface TranscriptSearchResult {
  mediaAssetId: string;
  assetTitle: string;
  language: string;
  matches: Array<{
    cueId: string;
    startTime: number;
    startTimeFormatted: string;
    text: string;
    snippet: string;
  }>;
}

@Injectable()
export class MediaService {
  constructor(
    @InjectModel(MediaAsset.name) private readonly mediaModel: Model<MediaAssetDocument>,
    @InjectModel('Course') private readonly courseModel: Model<any>,
  ) {}

  /**
   * Registers a new media asset for a course.
   */
  public async createMediaAsset(
    courseId: string,
    title: string,
    mediaUrl: string,
    mediaType: 'video' | 'audio' | 'document',
    durationSeconds = 0,
  ): Promise<MediaAsset> {
    const course = await this.courseModel.findById(courseId);
    if (!course) {
      throw new NotFoundException(`Course ${courseId} not found.`);
    }

    const asset = new this.mediaModel({
      courseId: new Types.ObjectId(courseId),
      title: title.trim(),
      mediaUrl,
      mediaType,
      durationSeconds,
      subtitles: [],
      chapters: [],
    });

    return await asset.save();
  }

  /**
   * Uploads and parses an SRT or WebVTT subtitle track for a media asset.
   */
  public async uploadSubtitleTrack(dto: UploadSubtitleDto): Promise<MediaAsset> {
    const asset = await this.mediaModel.findById(dto.mediaAssetId);
    if (!asset) {
      throw new NotFoundException(`Media asset ${dto.mediaAssetId} not found.`);
    }

    const parsedCues = SrtVttParserUtils.parseSubtitles(dto.subtitleContent);
    if (parsedCues.length === 0) {
      throw new BadRequestException('Failed to parse valid subtitle cues from input content.');
    }

    const lang = dto.language.trim().toLowerCase();
    const existingTrackIndex = asset.subtitles.findIndex(s => s.language === lang);

    if (existingTrackIndex >= 0) {
      asset.subtitles[existingTrackIndex].cues = parsedCues;
    } else {
      asset.subtitles.push({
        language: lang,
        cues: parsedCues,
      });
    }

    // Auto-generate chapter markers if video has no chapters yet
    if (asset.chapters.length === 0 && parsedCues.length > 0) {
      asset.chapters = SrtVttParserUtils.extractChapters(parsedCues);
    }

    return await asset.save();
  }

  /**
   * Performs full-text search across subtitle transcripts of a media asset.
   */
  public async searchTranscript(mediaAssetId: string, query: string): Promise<TranscriptSearchResult> {
    const asset = await this.mediaModel.findById(mediaAssetId);
    if (!asset) {
      throw new NotFoundException(`Media asset ${mediaAssetId} not found.`);
    }

    if (!query || query.trim().length < 2) {
      throw new BadRequestException('Search query must be at least 2 characters long.');
    }

    const cleanQuery = query.trim().toLowerCase();
    const matches: TranscriptSearchResult['matches'] = [];

    for (const track of asset.subtitles) {
      for (const cue of track.cues) {
        if (cue.text.toLowerCase().includes(cleanQuery)) {
          matches.push({
            cueId: cue.id,
            startTime: cue.startTime,
            startTimeFormatted: cue.startTimeFormatted,
            text: cue.text,
            snippet: this.highlightSnippet(cue.text, cleanQuery),
          });
        }
      }
    }

    return {
      mediaAssetId,
      assetTitle: asset.title,
      language: asset.subtitles.length > 0 ? asset.subtitles[0].language : 'en',
      matches,
    };
  }

  /**
   * Generates a signed stream playback URL token.
   */
  public async getStreamToken(userId: string, mediaAssetId: string): Promise<{ streamUrl: string; token: string; expiresAt: number }> {
    const asset = await this.mediaModel.findById(mediaAssetId);
    if (!asset) {
      throw new NotFoundException(`Media asset ${mediaAssetId} not found.`);
    }

    const tokenData = MediaTokenUtils.generateStreamToken(mediaAssetId, userId, 7200);
    const streamUrl = `${asset.mediaUrl}?token=${tokenData.token}`;

    return {
      streamUrl,
      token: tokenData.token,
      expiresAt: tokenData.expiresAt,
    };
  }

  /**
   * Lists all media assets for a course.
   */
  public async getCourseMediaAssets(courseId: string): Promise<MediaAsset[]> {
    return await this.mediaModel.find({ courseId: new Types.ObjectId(courseId) }).exec();
  }

  private highlightSnippet(text: string, query: string): string {
    const idx = text.toLowerCase().indexOf(query);
    if (idx === -1) return text;
    const start = Math.max(0, idx - 20);
    const end = Math.min(text.length, idx + query.length + 20);
    return (start > 0 ? '...' : '') + text.substring(start, end) + (end < text.length ? '...' : '');
  }
}
