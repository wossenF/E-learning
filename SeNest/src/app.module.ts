import { Module } from '@nestjs/common';
import { ConfigModule } from '@nestjs/config';
import { MongooseModule } from '@nestjs/mongoose';
import { AppController } from './app.controller';
import { AppService } from './app.service';
import { UserModule } from './user/user.module';
import { CoursesModule } from './courses/courses.module';
import { AuthModule } from './auth/auth.module';
import { AssessmentModule } from './assessment/assessment.module';
import { GradebookModule } from './gradebook/gradebook.module';
import { DiscussionModule } from './discussion/discussion.module';
import { NotificationsModule } from './notifications/notifications.module';
import { CertificatesModule } from './certificates/certificates.module';
import { MediaModule } from './media/media.module';
import { PlagiarismModule } from './plagiarism/plagiarism.module';

@Module({
  imports: [
    ConfigModule.forRoot({
      isGlobal: true,
      cache: true,
    }),
    MongooseModule.forRoot(process.env.DB_URI),
    UserModule,
    CoursesModule,
    AuthModule,
    AssessmentModule,
    GradebookModule,
    DiscussionModule,
    NotificationsModule,
    CertificatesModule,
    MediaModule,
    PlagiarismModule,
  ],
  controllers: [AppController],
  providers: [AppService],
})
export class AppModule {}
