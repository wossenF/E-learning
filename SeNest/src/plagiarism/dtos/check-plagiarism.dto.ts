export class CheckPlagiarismDto {
  targetSubmissionText: string;
  comparisonPool: Array<{
    submissionId: string;
    studentId: string;
    studentName?: string;
    content: string;
  }>;
}

export class PlagiarismMatchDto {
  submissionId: string;
  studentId: string;
  studentName: string;
  similarityScore: number;
  jaccardIndex: number;
  matchingNgramsCount: number;
  isFlagged: boolean;
}

export class PlagiarismReportDto {
  overallRiskLevel: 'low' | 'moderate' | 'high' | 'critical';
  highestSimilarityScore: number;
  matches: PlagiarismMatchDto[];
  analyzedAt: Date;
}
