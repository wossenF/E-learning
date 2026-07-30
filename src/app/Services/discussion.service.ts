import { Injectable } from '@angular/core';
import { HttpClient, HttpParams } from '@angular/common/http';
import { Observable } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class DiscussionService {
  private apiUrl = 'http://localhost:3000/discussion';

  constructor(private http: HttpClient) {}

  queryThreads(courseId: string, search?: string, tag?: string, sortBy?: string): Observable<any> {
    let params = new HttpParams().set('courseId', courseId);
    if (search) params = params.set('search', search);
    if (tag) params = params.set('tag', tag);
    if (sortBy) params = params.set('sortBy', sortBy);

    return this.http.get<any>(`${this.apiUrl}/threads`, { params });
  }

  getThreadDetails(threadId: string): Observable<any> {
    return this.http.get<any>(`${this.apiUrl}/threads/${threadId}`);
  }

  createThread(payload: { courseId: string; title: string; content: string; tags?: string[] }): Observable<any> {
    return this.http.post<any>(`${this.apiUrl}/threads`, payload);
  }

  postReply(payload: { threadId: string; content: string; parentReplyId?: string }): Observable<any> {
    return this.http.post<any>(`${this.apiUrl}/replies`, payload);
  }
}
