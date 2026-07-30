import { Prop, Schema, SchemaFactory } from '@nestjs/mongoose';
import { Document, Types } from 'mongoose';

export type MediaAssetDocument = MediaAsset & Document;

@Schema({ timestamps: true })
export class SubtitleTrack {
  @Prop({ required: true })
  language: string;

  @Prop({ type: Array, default: [] })
  cues: Array<{
    id: string;
    startTime: number;
    endTime: number;
    startTimeFormatted: string;
    endTimeFormatted: string;
    text: string;
  }>;
}

const SubtitleTrackSchema = SchemaFactory.createForClass(SubtitleTrack);

@Schema({ timestamps: true })
export class MediaAsset {
  @Prop({ type: Types.ObjectId, ref: 'Course', required: true })
  courseId: Types.ObjectId;

  @Prop({ required: true })
  title: string;

  @Prop({ required: true })
  mediaUrl: string;

  @Prop({ required: true, enum: ['video', 'audio', 'document'] })
  mediaType: string;

  @Prop({ default: 0 })
  durationSeconds: number;

  @Prop({ type: [SubtitleTrackSchema], default: [] })
  subtitles: SubtitleTrack[];

  @Prop({ type: Array, default: [] })
  chapters: Array<{ title: string; startTime: number }>;
}

export const MediaAssetSchema = SchemaFactory.createForClass(MediaAsset);
