// extra_big28.ts - scheduling and job runner (toy)
type Job = { id:string, run:()=>Promise<any>, intervalMs?:number };

export class Scheduler {
  private jobs = new Map<string, Job>();
  private timers = new Map<string, any>();
  addJob(job: Job) { this.jobs.set(job.id, job); if (job.intervalMs) this.startJob(job.id); }
  removeJob(id:string) { this.stopJob(id); this.jobs.delete(id); }
  startJob(id:string) { const job = this.jobs.get(id); if (!job || !job.intervalMs) return; if (this.timers.has(id)) return; const t = setInterval(()=> { job.run().catch(()=>{}); }, job.intervalMs); this.timers.set(id,t); }
  stopJob(id:string) { const t = this.timers.get(id); if (t) { clearInterval(t); this.timers.delete(id); } }
  async runOnce(id:string) { const job = this.jobs.get(id); if (!job) throw new Error('job not found'); return job.run(); }
}

export async function runSequence(jobs: Job[]) { for (const j of jobs) await j.run(); }
