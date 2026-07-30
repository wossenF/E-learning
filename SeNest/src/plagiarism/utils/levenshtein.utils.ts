export interface DiffChunk {
  type: 'equal' | 'insert' | 'delete';
  value: string;
}

export class LevenshteinUtils {
  /**
   * Calculates edit distance and similarity ratio between two source texts.
   */
  public static calculateDistance(source: string, target: string): { distance: number; similarityRatio: number } {
    const sLen = source.length;
    const tLen = target.length;

    if (sLen === 0) return { distance: tLen, similarityRatio: tLen === 0 ? 100 : 0 };
    if (tLen === 0) return { distance: sLen, similarityRatio: 0 };

    const matrix: number[][] = Array(tLen + 1).fill(null).map(() => Array(sLen + 1).fill(0));

    for (let i = 0; i <= sLen; i++) matrix[0][i] = i;
    for (let j = 0; j <= tLen; j++) matrix[j][0] = j;

    for (let j = 1; j <= tLen; j++) {
      for (let i = 1; i <= sLen; i++) {
        const cost = source[i - 1] === target[j - 1] ? 0 : 1;
        matrix[j][i] = Math.min(
          matrix[j][i - 1] + 1, // deletion
          matrix[j - 1][i] + 1, // insertion
          matrix[j - 1][i - 1] + cost, // substitution
        );
      }
    }

    const distance = matrix[tLen][sLen];
    const maxLen = Math.max(sLen, tLen);
    const similarityRatio = Math.round((1 - distance / maxLen) * 10000) / 100;

    return { distance, similarityRatio };
  }

  /**
   * Generates line-by-line unified diff chunks between two texts.
   */
  public static generateLineDiff(sourceText: string, targetText: string): DiffChunk[] {
    const sLines = sourceText.split('\n');
    const tLines = targetText.split('\n');
    const chunks: DiffChunk[] = [];

    const maxLines = Math.max(sLines.length, tLines.length);
    for (let i = 0; i < maxLines; i++) {
      const s = sLines[i];
      const t = tLines[i];

      if (s === t) {
        if (s !== undefined) chunks.push({ type: 'equal', value: s });
      } else {
        if (s !== undefined) chunks.push({ type: 'delete', value: s });
        if (t !== undefined) chunks.push({ type: 'insert', value: t });
      }
    }

    return chunks;
  }
}
