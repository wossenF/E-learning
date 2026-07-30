import { Module } from '@nestjs/common';
import { MongooseModule } from '@nestjs/mongoose';
import { GradebookController } from './gradebook.controller';
import { GradebookService } from './gradebook.service';
import { GradebookEntry, GradebookSchema } from './schemas/gradebook.schema';
import { CoursesSchema } from '../courses/Schemas/courses.schema';
import { UsersSchema } from '../user/Schemas/user.schema';

@Module({
  imports: [
    MongooseModule.forFeature([
      { name: GradebookEntry.name, schema: GradebookSchema },
      { name: 'Course', schema: CoursesSchema },
      { name: 'User', schema: UsersSchema },
    ]),
  ],
  controllers: [GradebookController],
  providers: [GradebookService],
  exports: [GradebookService],
})
export class GradebookModule {}
