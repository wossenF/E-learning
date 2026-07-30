import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs';

export interface QuizQuestion {
  questionId: string;
  title: string;
  type: string;
  options?: string[];
  points: number;
}

export interface Quiz {
  _id: string;
  title: string;
  description?: string;
  courseId: string;
  timeLimitMinutes: number;
  passPercentage: number;
  maxAttempts: number;
  questions: QuizQuestion[];
}

@Injectable({
  providedIn: 'root'
})
export class AssessmentService {
  private apiUrl = 'http://localhost:3000/assessments';

  constructor(private http: HttpClient) {}

  getQuiz(quizId: string): Observable<Quiz> {
    return this.http.get<Quiz>(`${this.apiUrl}/${quizId}`);
  }

  submitQuiz(quizId: string, payload: { timeSpentSeconds: number; answers: Array<{ questionId: string; userAnswer: any }> }): Observable<any> {
    return this.http.post<any>(`${this.apiUrl}/${quizId}/submit`, payload);
  }

  getQuizAnalytics(quizId: string): Observable<any> {
    return this.http.get<any>(`${this.apiUrl}/${quizId}/analytics`);
  }
}
