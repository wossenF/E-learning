# Temene — Full-Stack E-Learning Platform

A full-stack e-learning application with an **Angular 15 frontend** and a **NestJS + MongoDB backend**.

## Architecture

| Layer | Location | Tech |
|-------|----------|------|
| Frontend | `/src/` (root) | Angular 15, Bootstrap 5 |
| Backend  | `/SeNest/` | NestJS 9, Mongoose, Passport-JWT, bcrypt, Multer |

### Backend API (SeNest — port 3000)

| Route | Auth | Description |
|-------|------|-------------|
| `POST /auth/signup` | — | Register a new user |
| `POST /auth/login` | — | Login, returns JWT |
| `POST /auth/check` | — | Validate a JWT |
| `GET /courses` | — | List/search courses |
| `GET /courses/:id` | — | Get course detail |
| `POST /courses` | JWT | Create course |
| `PATCH /courses/:id` | JWT | Update course |
| `DELETE /courses/:id` | JWT | Delete course |
| `POST /courses/thumbnail/:id` | JWT | Upload thumbnail |
| `GET /users` | JWT | List all users |
| `GET /users/detail` | JWT | Get own profile |
| `PATCH /users` | JWT | Update own profile |
| `DELETE /users` | JWT | Delete own account |
| `POST /users/enroll` | JWT | Enroll in a course |
| `POST /users/profile_picture` | JWT | Upload profile picture |

### Frontend (Angular — port 4200)

- Auth: login, register
- Course browsing, search/filter, detail, enrollment
- Instructor dashboard: create course, manage students
- Profile management

## Running Locally

### Backend

```bash
cd SeNest
npm install
# Set environment variable: JWT_SECRET=<at-least-32-chars>
# Set environment variable: DB_URI=<mongodb-connection-string>
npm run start:dev
```

### Frontend

```bash
npm install
npm start
# Navigates to http://localhost:4200
```

## Required Environment Variables

| Variable | Where | Description |
|----------|-------|-------------|
| `DB_URI` | SeNest | MongoDB connection string |
| `JWT_SECRET` | SeNest | JWT signing secret (≥ 32 chars) |

The app will crash at startup without both variables set.

## Security Analysis

See `SECURITY.md` for the full vulnerability report.  
See `fuzz/` for all fuzzing harnesses, PoCs, and ClusterFuzzLite configuration.

## User Preferences

- Keep the existing Angular + NestJS monorepo structure.
- Do not migrate the database away from MongoDB/Mongoose.
