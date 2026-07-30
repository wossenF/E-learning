import { Injectable } from '@angular/core';
import { HttpClient } from '@angular/common/http';
import { Observable } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class CertificateService {
  private apiUrl = 'http://localhost:3000/certificates';

  constructor(private http: HttpClient) {}

  getMyCertificates(): Observable<any[]> {
    return this.http.get<any[]>(`${this.apiUrl}/my-certificates`);
  }

  getMyBadges(): Observable<any[]> {
    return this.http.get<any[]>(`${this.apiUrl}/my-badges`);
  }

  verifyCertificate(code: string): Observable<any> {
    return this.http.get<any>(`${this.apiUrl}/verify/${code}`);
  }
}
