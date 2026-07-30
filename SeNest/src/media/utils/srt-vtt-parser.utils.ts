export interface SubtitleCue {
  id: string;
  startTime: number; // in seconds
  endTime: number; // in seconds
  startTimeFormatted: string;
  endTimeFormatted: string;
  text: string;
}

export class SrtVttParserUtils {
  /**
   * Parses SRT or WebVTT format text content into structured SubtitleCue items.
   */
  public static parseSubtitles(content: string): SubtitleCue[] {
    if (!content) return [];

    const cues: SubtitleCue[] = [];
    const normalized = content.replace(/\r\n/g, '\n').replace(/\r/g, '\n');
    const blocks = normalized.split(/\n\n+/);

    let idx = 1;
    for (const block of blocks) {
      const lines = block.trim().split('\n').map(l => l.trim()).filter(Boolean);
      if (lines.length === 0) continue;

      // Skip WEBVTT header line if present
      if (lines[0].toUpperCase().startsWith('WEBVTT') || lines[0].toUpperCase().startsWith('NOTE')) {
        continue;
      }

      let timeLineIdx = 0;
      let cueId = `cue_${idx++}`;

      if (/^\d+$/.test(lines[0])) {
        cueId = lines[0];
        timeLineIdx = 1;
      }

      if (timeLineIdx >= lines.length) continue;

      const timeLine = lines[timeLineIdx];
      const match = timeLine.match(/(\d{1,2}:\d{2}:\d{2}[\.,]\d{3}|\d{2}:\d{2}[\.,]\d{3})\s*-->\s*(\d{1,2}:\d{2}:\d{2}[\.,]\d{3}|\d{2}:\d{2}[\.,]\d{3})/);

      if (!match) continue;

      const startTimeStr = match[1];
      const endTimeStr = match[2];

      const startTime = this.timeToSeconds(startTimeStr);
      const endTime = this.timeToSeconds(endTimeStr);

      const textLines = lines.slice(timeLineIdx + 1);
      const text = textLines.join(' ').replace(/<[^>]*>/g, '').trim();

      if (text) {
        cues.push({
          id: cueId,
          startTime,
          endTime,
          startTimeFormatted: startTimeStr,
          endTimeFormatted: endTimeStr,
          text,
        });
      }
    }

    return cues;
  }

  /**
   * Converts HH:MM:SS,mmm or MM:SS.mmm string to seconds integer/float.
   */
  public static timeToSeconds(timeStr: string): number {
    const parts = timeStr.replace(',', '.').split(':');
    let seconds = 0;

    if (parts.length === 3) {
      seconds += parseInt(parts[0], 10) * 3600;
      seconds += parseInt(parts[1], 10) * 60;
      seconds += parseFloat(parts[2]);
    } else if (parts.length === 2) {
      seconds += parseInt(parts[0], 10) * 60;
      seconds += parseFloat(parts[1]);
    }

    return Math.round(seconds * 1000) / 1000;
  }

  /**
   * Generates video chapters from subtitle cue headings or silence gaps.
   */
  public static extractChapters(cues: SubtitleCue[]): Array<{ title: string; startTime: number }> {
    const chapters: Array<{ title: string; startTime: number }> = [];
    if (cues.length === 0) return chapters;

    chapters.push({ title: 'Introduction', startTime: cues[0].startTime });

    for (let i = 1; i < cues.length; i++) {
      const prev = cues[i - 1];
      const curr = cues[i];

      // Chapter boundary detected if there's a > 5 second gap or explicit chapter text
      const gap = curr.startTime - prev.endTime;
      const isChapterKeyword = /^chapter\s+\d+|\[chapter\]|^section\s+\d+/i.test(curr.text);

      if (gap >= 5.0 || isChapterKeyword) {
        const title = isChapterKeyword ? curr.text : `Section ${chapters.length + 1}`;
        chapters.push({ title, startTime: curr.startTime });
      }
    }

    return chapters;
  }
}
