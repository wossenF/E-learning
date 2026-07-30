import { Module } from '@nestjs/common';
import { MongooseModule } from '@nestjs/mongoose';
import { NotificationsController } from './notifications.controller';
import { NotificationsService } from './notifications.service';
import { Notification, NotificationSchema } from './schemas/notification.schema';
import { Announcement, AnnouncementSchema } from './schemas/announcement.schema';
import { UsersSchema } from '../user/Schemas/user.schema';
import { CoursesSchema } from '../courses/Schemas/courses.schema';

@Module({
  imports: [
    MongooseModule.forFeature([
      { name: Notification.name, schema: NotificationSchema },
      { name: Announcement.name, schema: AnnouncementSchema },
      { name: 'User', schema: UsersSchema },
      { name: 'Course', schema: CoursesSchema },
    ]),
  ],
  controllers: [NotificationsController],
  providers: [NotificationsService],
  exports: [NotificationsService],
})
export class NotificationsModule {}
