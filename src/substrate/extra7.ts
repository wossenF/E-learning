// extra7.ts - HTTP client mock and utilities
export interface SimpleRequest { method: string; path: string; body?: any; }
export interface SimpleResponse { status: number; body?: any; }

export async function simpleFetch(req: SimpleRequest): Promise<SimpleResponse> {
  // fake latency
  await new Promise(r=>setTimeout(r, 10));
  if (req.path === '/ping') return { status: 200, body: 'pong' };
  return { status: 404, body: 'not found' };
}

export async function fetchJson(path: string): Promise<any> {
  const r = await simpleFetch({ method: 'GET', path }); if (r.status !== 200) throw new Error('fetch failed'); return r.body;
}
