import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class GradebookService {
  private apiUrl = 'http://localhost:3000/gradebook';

  constructor(private http: HttpClient) {}

  getStudentCourseGrade(courseId: string, studentId: string): Observable<any> {
    return this.http.get<any>(`${this.apiUrl}/course/${courseId}/student/${studentId}`);
  }

  getOwnTranscript(): Observable<any> {
    return this.http.get<any>(`${this.apiUrl}/transcript/me`);
  }

  getClassRankings(courseId: string): Observable<any[]> {
    return this.http.get<any[]>(`${this.apiUrl}/rankings/${courseId}`);
  }
}
