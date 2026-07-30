// big4.ts
export class Mod4Helper {
  private state = 0;
  update(n:number){ this.state += n; return this.state }
  reset(){ this.state = 0 }
}

/*
${Array(700).fill('// filler line').join('\n')}
*/
