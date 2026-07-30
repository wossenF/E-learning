export class NgramUtils {
  /**
   * Tokenizes input code or text into normalized word/code tokens.
   */
  public static tokenize(text: string): string[] {
    if (!text) return [];
    return text
      .toLowerCase()
      .replace(/[^\w\s]/g, ' ') // Strip punctuation
      .split(/\s+/)
      .filter(t => t.length > 1);
  }

  /**
   * Extracts character or word n-grams from a list of tokens.
   */
  public static extractNgrams(tokens: string[], n = 3): string[] {
    if (tokens.length < n) return [];
    const ngrams: string[] = [];
    for (let i = 0; i <= tokens.length - n; i++) {
      ngrams.push(tokens.slice(i, i + n).join('_'));
    }
    return ngrams;
  }

  /**
   * Computes Jaccard Similarity index between two sets of n-grams (0.0 to 1.0).
   */
  public static jaccardSimilarity(setA: Set<string>, setB: Set<string>): number {
    if (setA.size === 0 && setB.size === 0) return 1.0;
    if (setA.size === 0 || setB.size === 0) return 0.0;

    let intersectionCount = 0;
    for (const item of setA) {
      if (setB.has(item)) intersectionCount++;
    }

    const unionCount = setA.size + setB.size - intersectionCount;
    return Math.round((intersectionCount / unionCount) * 10000) / 100;
  }
}
