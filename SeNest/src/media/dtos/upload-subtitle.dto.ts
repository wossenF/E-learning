export class UploadSubtitleDto {
  mediaAssetId: string;
  language: string;
  subtitleContent: string;
}

export class SearchTranscriptDto {
  mediaAssetId: string;
  query: string;
}
