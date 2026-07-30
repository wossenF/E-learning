// extra_big4.ts - file and path helpers (fake-friendly)
export function basename(path: string): string { const parts = path.split('/'); return parts[parts.length-1]; }
export function dirname(path: string): string { const parts = path.split('/'); parts.pop(); return parts.join('/') || '.'; }

export function joinPath(...parts: string[]): string { return parts.join('/').replace(/\/+/g,'/'); }

export function extname(path: string): string { const b = basename(path); const idx = b.lastIndexOf('.'); return idx===-1? '': b.substr(idx); }

export function isAbsolute(path: string): boolean { return path.startsWith('/') || /^[A-Za-z]:\\/.test(path); }

export function normalizePath(path: string): string { return path.replace(/\/+/g,'/').replace(/\/./g,''); }

export function stripExt(path: string): string { const e = extname(path); return e? path.substr(0, path.length - e.length) : path; }

export function ensureSlash(path: string): string { return path.endsWith('/')? path : path+'/'; }
