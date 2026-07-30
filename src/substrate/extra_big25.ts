// extra_big25.ts - random data generators
import { Row } from './extra_big16';

export function randomInt(min:number, max:number): number { return Math.floor(Math.random()*(max-min+1))+min; }
export function randomChoice<T>(arr:T[]): T { return arr[Math.floor(Math.random()*arr.length)]; }

export function genUsers(n:number): Row[] {
  const names = ['Alice','Bob','Carol','Dave','Eve','Frank','Grace','Heidi','Ivan','Judy'];
  const out: Row[] = [];
  for (let i=0;i<n;++i) out.push({ id: i, name: randomChoice(names), age: randomInt(18,80), active: Math.random()>0.5 });
  return out;
}

export function genTransactions(n:number): Row[] {
  const out: Row[] = []; for (let i=0;i<n;++i) out.push({ id:i, userId: randomInt(0,100), amount: (Math.random()*1000).toFixed(2), ts: Date.now()+i }); return out;
}

export function shuffle<T>(arr:T[]): T[] { const a = arr.slice(); for (let i=a.length-1;i>0;--i) { const j = Math.floor(Math.random()*(i+1)); [a[i],a[j]]=[a[j],a[i]]; } return a; }

export function range(n:number): number[] { const out:number[]=[]; for (let i=0;i<n;++i) out.push(i); return out; }
