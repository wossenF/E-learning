// extra_big29.ts - HTTP client/stub utilities
export async function httpGet(url:string, timeoutMs=5000): Promise<{status:number, body:string}> {
  // lightweight fetch wrapper; uses node-fetch when available
  try {
    const fetch = (global as any).fetch || (await import('node-fetch')).default;
    const res = await fetch(url, { method: 'GET' }); const text = await res.text(); return { status: res.status || 200, body: text };
  } catch(e:any) { return { status: 0, body: String(e.message || e) }; }
}

export async function httpPost(url:string, body:any): Promise<{status:number, body:string}> {
  try { const fetch = (global as any).fetch || (await import('node-fetch')).default; const res = await fetch(url, { method:'POST', body: JSON.stringify(body), headers: {'content-type':'application/json'} }); const text = await res.text(); return { status: res.status||200, body: text }; } catch(e:any) { return { status:0, body: String(e.message||e) }; }
}

export function parseQueryString(q:string): Record<string,string> { const out:any={}; for (const part of q.replace(/^\?/,'').split('&')) { if (!part) continue; const [k,v]=part.split('='); out[decodeURIComponent(k)] = decodeURIComponent(v||''); } return out; }
