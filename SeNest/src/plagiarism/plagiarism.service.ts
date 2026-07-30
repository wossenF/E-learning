import { Injectable, BadRequestException } from '@nestjs/common';
import { CheckPlagiarismDto, PlagiarismReportDto, PlagiarismMatchDto } from './dtos/check-plagiarism.dto';
import { NgramUtils } from './utils/ngram.utils';
import { LevenshteinUtils, DiffChunk } from './utils/levenshtein.utils';

@Injectable()
export class PlagiarismService {
  /**
   * Compares a target submission against a pool of student submissions using N-gram Jaccard similarity and Levenshtein edit distance.
   */
  public analyzePlagiarism(dto: CheckPlagiarismDto): PlagiarismReportDto {
    if (!dto.targetSubmissionText || dto.targetSubmissionText.trim().length < 10) {
      throw new BadRequestException('Target submission text must be at least 10 characters long for analysis.');
    }

    if (!dto.comparisonPool || dto.comparisonPool.length === 0) {
      return {
        overallRiskLevel: 'low',
        highestSimilarityScore: 0,
        matches: [],
        analyzedAt: new Date(),
      };
    }

    const targetTokens = NgramUtils.tokenize(dto.targetSubmissionText);
    const targetNgrams = new Set(NgramUtils.extractNgrams(targetTokens, 3));

    const matches: PlagiarismMatchDto[] = [];
    let highestScore = 0;

    for (const comp of dto.comparisonPool) {
      const compTokens = NgramUtils.tokenize(comp.content);
      const compNgrams = new Set(NgramUtils.extractNgrams(compTokens, 3));

      const jaccardScore = NgramUtils.jaccardSimilarity(targetNgrams, compNgrams);
      const levenshtein = LevenshteinUtils.calculateDistance(dto.targetSubmissionText, comp.content);

      // Weighted similarity score (60% N-gram Jaccard, 40% Levenshtein Ratio)
      const combinedScore = Math.round((jaccardScore * 0.6 + levenshtein.similarityRatio * 0.4) * 100) / 100;

      if (combinedScore > highestScore) {
        highestScore = combinedScore;
      }

      let matchingNgramsCount = 0;
      for (const ng of targetNgrams) {
        if (compNgrams.has(ng)) matchingNgramsCount++;
      }

      matches.push({
        submissionId: comp.submissionId,
        studentId: comp.studentId,
        studentName: comp.studentName || `Student ${comp.studentId}`,
        similarityScore: combinedScore,
        jaccardIndex: jaccardScore,
        matchingNgramsCount,
        isFlagged: combinedScore >= 60.0,
      });
    }

    matches.sort((a, b) => b.similarityScore - a.similarityScore);

    let overallRiskLevel: PlagiarismReportDto['overallRiskLevel'] = 'low';
    if (highestScore >= 80) overallRiskLevel = 'critical';
    else if (highestScore >= 60) overallRiskLevel = 'high';
    else if (highestScore >= 35) overallRiskLevel = 'moderate';

    return {
      overallRiskLevel,
      highestSimilarityScore: highestScore,
      matches,
      analyzedAt: new Date(),
    };
  }

  /**
   * Generates a line-by-line unified diff between target submission and a comparison document.
   */
  public generateSubmissionDiff(sourceText: string, targetText: string): { diffChunks: DiffChunk[]; matchPct: number } {
    const lev = LevenshteinUtils.calculateDistance(sourceText, targetText);
    const chunks = LevenshteinUtils.generateLineDiff(sourceText, targetText);

    return {
      diffChunks: chunks,
      matchPct: lev.similarityRatio,
    };
  }
}
