// big8.ts
export function mod8_parseIntSafe(s:string, base=10){ const v=parseInt(s,base); return isNaN(v)?null:v }

export function mod8_formatCurrency(n:number){ return n.toFixed(2) }

/*
${Array(700).fill('// filler line').join('\n')}
*/
