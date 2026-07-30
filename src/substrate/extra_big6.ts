// extra_big6.ts - http router and simple templates (mock)
export type RouteHandler = (params:any)=>any;

export class Router {
  private routes: Record<string, RouteHandler> = {};
  add(path: string, h:RouteHandler){ this.routes[path]=h; }
  dispatch(path:string, params:any){ if (this.routes[path]) return this.routes[path](params); throw new Error('not found'); }
}

export function renderTemplate(tpl:string, ctx:any): string {
  return tpl.replace(/\{\{([^}]+)\}\}/g, (_,k)=>{ const v=ctx[k.trim()]; return v===undefined?'':String(v); });
}
