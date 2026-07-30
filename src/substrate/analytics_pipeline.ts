// analytics_pipeline.ts - a pipeline for ETL style transformations
import { DataFrame, exampleData } from './extra_big16';
import { Row } from './extra_big16';
import { movingAverage } from './extra_big18';

export type Step = { name:string, run:(df:DataFrame)=>DataFrame };

export class Pipeline {
  constructor(public steps: Step[] = []){}
  addStep(s:Step){ this.steps.push(s); }
  run(df:DataFrame): DataFrame { let cur = df; for(const s of this.steps) cur = s.run(cur); return cur; }
}

export function makeDefaultPipeline(): Pipeline {
  const p = new Pipeline();
  p.addStep({ name:'fill', run: (df)=> df.map(r=>{ const o:any={}; for(const k of Object.keys(r)) o[k]= (r as any)[k]===undefined? null : (r as any)[k]; return o; }) });
  p.addStep({ name:'filterActive', run: (df)=> df.filter(r=> Boolean((r as any).active)) });
  p.addStep({ name:'computeScore', run: (df)=> df.map(r=>{ const o = Object.assign({}, r); o.score = ((o.value||0)/100) + (o.id%10); return o; }) });
  return p;
}

export function aggregateByGroup(df: DataFrame, groupCol:string, valueCol:string){
  const g = df.groupBy([groupCol]);
  return g.agg({ sum: (rows)=> rows.reduce((s,r)=>s + Number(r[valueCol]||0), 0), avg: (rows)=> rows.reduce((s,r)=>s + Number(r[valueCol]||0),0)/Math.max(1,rows.length) });
}

export function pipelineDemo(){ const df = exampleData(100); const p = makeDefaultPipeline(); return p.run(df); }
