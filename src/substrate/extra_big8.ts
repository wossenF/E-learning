// extra_big8.ts - feature toggles and config helpers
export function envOr(key:string, def:string): string { const v=(process && (process.env as any)[key])||def; return v; }

export function parseBool(s:string|undefined, def=false): boolean { if (s===undefined) return def; const v=s.toLowerCase(); return v==='1'||v==='true'||v==='yes'; }

export function buildConfig(overrides:Record<string,any>) { const base = { host:'localhost', port:3000, debug:false }; return Object.assign(base, overrides); }

export function isFeatureEnabled(name:string, cfg:any): boolean { return !!cfg.features && !!cfg.features[name]; }
