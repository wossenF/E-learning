import { BadRequestException } from '@nestjs/common';

type MilestoneLike = {
  id: string;
  prerequisiteMilestoneIds?: string[];
  requiredCourseIds?: string[];
  optionalCourseIds?: string[];
  projects?: { estimatedMinutes?: number }[];
  estimatedMinutes?: number;
  unlockWeight?: number;
};

export type PathHealthReport = {
  milestoneCount: number;
  hasCycles: boolean;
  duplicateMilestoneIds: string[];
  orphanPrerequisites: string[];
  rootMilestones: string[];
  terminalMilestones: string[];
  longestPathLength: number;
  estimatedTotalMinutes: number;
  weightedComplexity: number;
};

export function ensureMilestoneGraphIsValid(milestones: MilestoneLike[]): void {
  const byId = new Map<string, MilestoneLike>();
  const duplicates: string[] = [];

  for (const milestone of milestones) {
    if (!milestone.id || milestone.id.trim().length === 0) {
      throw new BadRequestException('Each milestone must define a non-empty id.');
    }
    if (byId.has(milestone.id)) {
      duplicates.push(milestone.id);
      continue;
    }
    byId.set(milestone.id, milestone);
  }

  if (duplicates.length > 0) {
    throw new BadRequestException(`Duplicate milestone ids found: ${duplicates.join(', ')}`);
  }

  for (const milestone of milestones) {
    for (const prereq of milestone.prerequisiteMilestoneIds ?? []) {
      if (!byId.has(prereq)) {
        throw new BadRequestException(
          `Milestone ${milestone.id} references missing prerequisite milestone ${prereq}.`,
        );
      }
    }
  }

  if (hasCycle(milestones)) {
    throw new BadRequestException('Milestone prerequisite graph contains a cycle.');
  }
}

export function hasCycle(milestones: MilestoneLike[]): boolean {
  const state = new Map<string, 0 | 1 | 2>();
  const byId = new Map<string, MilestoneLike>(milestones.map((m) => [m.id, m]));

  const dfs = (id: string): boolean => {
    const current = state.get(id) ?? 0;
    if (current === 1) {
      return true;
    }
    if (current === 2) {
      return false;
    }
    state.set(id, 1);
    const node = byId.get(id);
    for (const prereqId of node?.prerequisiteMilestoneIds ?? []) {
      if (!byId.has(prereqId)) {
        continue;
      }
      if (dfs(prereqId)) {
        return true;
      }
    }
    state.set(id, 2);
    return false;
  };

  for (const milestone of milestones) {
    if (dfs(milestone.id)) {
      return true;
    }
  }
  return false;
}

export function buildPathHealthReport(milestones: MilestoneLike[]): PathHealthReport {
  const byId = new Map<string, MilestoneLike>();
  const duplicates: string[] = [];
  const orphanPrerequisites: string[] = [];

  for (const milestone of milestones) {
    if (!byId.has(milestone.id)) {
      byId.set(milestone.id, milestone);
    } else {
      duplicates.push(milestone.id);
    }
  }

  const indegree = new Map<string, number>();
  const outgoing = new Map<string, string[]>();
  for (const milestone of milestones) {
    indegree.set(milestone.id, indegree.get(milestone.id) ?? 0);
    outgoing.set(milestone.id, outgoing.get(milestone.id) ?? []);
  }

  for (const milestone of milestones) {
    for (const prereqId of milestone.prerequisiteMilestoneIds ?? []) {
      if (!byId.has(prereqId)) {
        orphanPrerequisites.push(`${milestone.id}->${prereqId}`);
        continue;
      }
      indegree.set(milestone.id, (indegree.get(milestone.id) ?? 0) + 1);
      const out = outgoing.get(prereqId) ?? [];
      out.push(milestone.id);
      outgoing.set(prereqId, out);
    }
  }

  const roots = [...indegree.entries()]
    .filter(([, degree]) => degree === 0)
    .map(([id]) => id)
    .sort();

  const terminals = [...outgoing.entries()]
    .filter(([, deps]) => deps.length === 0)
    .map(([id]) => id)
    .sort();

  const cycle = hasCycle(milestones);
  const longestPathLength = computeLongestPathLength(milestones);
  const estimatedTotalMinutes = computeEstimatedTotalMinutes(milestones);
  const weightedComplexity = computeWeightedComplexity(milestones);

  return {
    milestoneCount: milestones.length,
    hasCycles: cycle,
    duplicateMilestoneIds: [...new Set(duplicates)],
    orphanPrerequisites: [...new Set(orphanPrerequisites)],
    rootMilestones: roots,
    terminalMilestones: terminals,
    longestPathLength,
    estimatedTotalMinutes,
    weightedComplexity,
  };
}

export function computeLongestPathLength(milestones: MilestoneLike[]): number {
  const byId = new Map<string, MilestoneLike>(milestones.map((m) => [m.id, m]));
  const cache = new Map<string, number>();

  const depth = (id: string): number => {
    if (cache.has(id)) {
      return cache.get(id) as number;
    }
    const node = byId.get(id);
    const prereqs = node?.prerequisiteMilestoneIds ?? [];
    if (prereqs.length === 0) {
      cache.set(id, 1);
      return 1;
    }
    const value =
      1 +
      prereqs
        .filter((prereqId) => byId.has(prereqId))
        .reduce((maxDepth, prereqId) => Math.max(maxDepth, depth(prereqId)), 0);
    cache.set(id, value);
    return value;
  };

  let longest = 0;
  for (const milestone of milestones) {
    longest = Math.max(longest, depth(milestone.id));
  }
  return longest;
}

export function computeEstimatedTotalMinutes(milestones: MilestoneLike[]): number {
  return milestones.reduce((sum, milestone) => {
    const base = milestone.estimatedMinutes ?? 0;
    const projectMinutes = (milestone.projects ?? []).reduce(
      (projectSum, project) => projectSum + (project.estimatedMinutes ?? 0),
      0,
    );
    return sum + base + projectMinutes;
  }, 0);
}

export function computeWeightedComplexity(milestones: MilestoneLike[]): number {
  const totalWeight = milestones.reduce((sum, milestone) => sum + (milestone.unlockWeight ?? 1), 0);
  if (totalWeight <= 0) {
    return 0;
  }

  let weightedMinutes = 0;
  for (const milestone of milestones) {
    const weight = milestone.unlockWeight ?? 1;
    const minutes = (milestone.estimatedMinutes ?? 0) +
      (milestone.projects ?? []).reduce((acc, p) => acc + (p.estimatedMinutes ?? 0), 0);
    const prereqFactor = Math.max(1, (milestone.prerequisiteMilestoneIds ?? []).length);
    const courseFactor =
      1 +
      ((milestone.requiredCourseIds ?? []).length * 0.2) +
      ((milestone.optionalCourseIds ?? []).length * 0.1);

    weightedMinutes += minutes * weight * prereqFactor * courseFactor;
  }

  return Math.round(weightedMinutes / totalWeight);
}

export function calculateDiscoverabilityScore(input: {
  title: string;
  summary: string;
  tags: string[];
  competencies: string[];
  outcomes: string[];
  featured: boolean;
  localeAvailability: string[];
}): number {
  const titleWords = input.title.trim().split(/\s+/).filter(Boolean).length;
  const summaryWords = input.summary.trim().split(/\s+/).filter(Boolean).length;

  let score = 0;
  score += Math.min(20, titleWords * 2);
  score += Math.min(20, Math.floor(summaryWords / 4));
  score += Math.min(15, input.tags.length * 2);
  score += Math.min(15, input.competencies.length * 2);
  score += Math.min(10, input.outcomes.length * 2);
  score += Math.min(10, input.localeAvailability.length * 2);
  score += input.featured ? 10 : 0;

  return Math.max(0, Math.min(100, score));
}

export function calculateQualityScore(input: {
  milestones: MilestoneLike[];
  completionPolicy?: {
    minQuizScore?: number;
    minProjectRubricScore?: number;
    minAttendance?: number;
    maxInactivityDays?: number;
    requiresCoachSignoff?: boolean;
  };
  outcomes: string[];
  capstoneRequirements: string[];
}): number {
  const milestoneCount = input.milestones.length;
  const depth = computeLongestPathLength(input.milestones);
  const totalMinutes = computeEstimatedTotalMinutes(input.milestones);

  let score = 0;
  score += Math.min(20, milestoneCount * 2);
  score += Math.min(20, depth * 3);
  score += Math.min(20, Math.floor(totalMinutes / 120));
  score += Math.min(15, input.outcomes.length * 2);
  score += Math.min(10, input.capstoneRequirements.length * 2);

  const policy = input.completionPolicy ?? {};
  if ((policy.minQuizScore ?? 0) >= 60) {
    score += 5;
  }
  if ((policy.minProjectRubricScore ?? 0) >= 60) {
    score += 5;
  }
  if ((policy.minAttendance ?? 0) >= 70) {
    score += 3;
  }
  if ((policy.maxInactivityDays ?? 30) <= 10) {
    score += 1;
  }
  if (policy.requiresCoachSignoff) {
    score += 1;
  }

  return Math.max(0, Math.min(100, score));
}
