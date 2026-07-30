// extra_big35.ts - logging utilities
export enum LogLevel { DEBUG=10, INFO=20, WARN=30, ERROR=40 }

export class Logger {
  constructor(private level=LogLevel.INFO, private prefix='') {}
  private should(level:LogLevel) { return level>=this.level; }
  debug(...args:any[]) { if (this.should(LogLevel.DEBUG)) console.debug(this.prefix, ...args); }
  info(...args:any[]) { if (this.should(LogLevel.INFO)) console.info(this.prefix, ...args); }
  warn(...args:any[]) { if (this.should(LogLevel.WARN)) console.warn(this.prefix, ...args); }
  error(...args:any[]) { if (this.should(LogLevel.ERROR)) console.error(this.prefix, ...args); }
}

export function createLogger(prefix:string, level=LogLevel.INFO) { return new Logger(level, prefix); }
