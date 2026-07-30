// sql_optimizer.ts - simple logical->physical planner with rewrite rules
import { Row } from './extra_big16';

export type LogicalPlan = { type: 'scan'|'filter'|'project'|'aggregate'|'limit'|'join', table?:string, expr?:any, cols?:string[], left?:LogicalPlan, right?:LogicalPlan, group?:string[] };
export type PhysicalPlan = { op: string, args?: any };

export function pushDownFilters(plan: LogicalPlan): LogicalPlan {
  // For join nodes, try to push filter into children if predicate references one side only
  if (plan.type === 'filter' && plan.left) {
    const pred = plan.expr;
    const child = plan.left;
    if (child.type === 'join') {
      const refs = referencedTables(pred);
      if (refs.size === 1 && child.left && child.right) {
        const target = refs.has(child.left.table||'') ? 'left' : 'right';
        if (target === 'left') child.left = { type: 'filter', expr: pred, left: child.left } as LogicalPlan;
        else child.right = { type: 'filter', expr: pred, left: child.right } as LogicalPlan;
        return child;
      }
    }
  }
  // recurse
  if (plan.left) plan.left = pushDownFilters(plan.left);
  if (plan.right) plan.right = pushDownFilters(plan.right);
  return plan;
}

function referencedTables(expr:any): Set<string> {
  const out = new Set<string>();
  if (!expr) return out;
  const stack = [expr];
  while (stack.length) {
    const e = stack.pop();
    if (!e) continue;
    if (e.type === 'col' && e.table) out.add(e.table);
    if (e.left) stack.push(e.left);
    if (e.right) stack.push(e.right);
    if (e.args) for (const a of e.args) stack.push(a);
  }
  return out;
}

export function projectPruning(plan: LogicalPlan, needed:Set<string>|null=null): LogicalPlan {
  if (!needed) needed = new Set(plan.cols || []);
  if (plan.type === 'project') {
    needed = new Set(plan.cols || []);
    if (plan.left) plan.left = projectPruning(plan.left, needed);
    return plan;
  }
  if (plan.left) plan.left = projectPruning(plan.left, needed);
  if (plan.right) plan.right = projectPruning(plan.right, needed);
  return plan;
}

export function logicalToPhysical(plan: LogicalPlan): PhysicalPlan {
  plan = pushDownFilters(plan);
  plan = projectPruning(plan);
  // simple mapping
  if (plan.type === 'scan') return { op: 'table_scan', args: { table: plan.table } };
  if (plan.type === 'filter' && plan.left) return { op: 'filter', args: { pred: plan.expr, child: logicalToPhysical(plan.left) } };
  if (plan.type === 'project' && plan.left) return { op: 'project', args: { cols: plan.cols, child: logicalToPhysical(plan.left) } };
  if (plan.type === 'aggregate' && plan.left) return { op: 'aggregate', args: { group: plan.group, child: logicalToPhysical(plan.left) } };
  if (plan.type === 'limit' && plan.left) return { op: 'limit', args: { n: plan.expr, child: logicalToPhysical(plan.left) } };
  if (plan.type === 'join' && plan.left && plan.right) return { op: 'nested_loop_join', args: { left: logicalToPhysical(plan.left), right: logicalToPhysical(plan.right), expr: plan.expr } };
  return { op: 'unknown' };
}

// Small cost model for demonstration
export function estimateCost(p: PhysicalPlan): number {
  if (!p) return 1000;
  switch (p.op) {
    case 'table_scan': return 100;
    case 'filter': return 10 + estimateCost(p.args.child);
    case 'project': return 1 + estimateCost(p.args.child);
    case 'aggregate': return 50 + estimateCost(p.args.child);
    case 'limit': return 1 + estimateCost(p.args.child);
    case 'nested_loop_join': return estimateCost(p.args.left) * estimateCost(p.args.right) / 10;
    default: return 1000;
  }
}

// simple optimizer that enumerates a couple of plans and picks lowest cost
export function optimize(plan: LogicalPlan): PhysicalPlan {
  const p1 = logicalToPhysical(plan);
  const cost1 = estimateCost(p1);
  // alternative: swap join sides
  let p2 = p1;
  if (p1.op === 'nested_loop_join') {
    p2 = { op: 'nested_loop_join', args: { left: p1.args.right, right: p1.args.left, expr: p1.args.expr } };
  }
  const cost2 = estimateCost(p2);
  return cost2 < cost1 ? p2 : p1;
}

// utility: pretty print plan
export function prettyPlan(p: PhysicalPlan, indent=0): string {
  const pad = ' '.repeat(indent);
  if (!p) return pad + 'null';
  let s = pad + p.op;
  if (p.args) {
    s += ' ' + JSON.stringify(Object.keys(p.args).filter(k=>k!=='child' && k!=='left' && k!=='right'));
    if (p.args.child) s += '\n' + prettyPlan(p.args.child, indent+2);
    if (p.args.left) s += '\n' + prettyPlan(p.args.left, indent+2);
    if (p.args.right) s += '\n' + prettyPlan(p.args.right, indent+2);
  }
  return s;
}

// small tests
export function selfTest(): boolean {
  const plan: LogicalPlan = { type: 'select', cols: ['id','name'], left: { type: 'scan', table: 'users' } } as any;
  const phys = optimize(plan);
  return !!phys && typeof estimateCost(phys) === 'number';
}
