import { Module } from '@nestjs/common';
import { MongooseModule } from '@nestjs/mongoose';
import { DiscussionController } from './discussion.controller';
import { DiscussionService } from './discussion.service';
import { DiscussionThread, DiscussionThreadSchema } from './schemas/discussion-thread.schema';
import { Reply, ReplySchema } from './schemas/reply.schema';
import { UsersSchema } from '../user/Schemas/user.schema';

@Module({
  imports: [
    MongooseModule.forFeature([
      { name: DiscussionThread.name, schema: DiscussionThreadSchema },
      { name: Reply.name, schema: ReplySchema },
      { name: 'User', schema: UsersSchema },
    ]),
  ],
  controllers: [DiscussionController],
  providers: [DiscussionService],
  exports: [DiscussionService],
})
export class DiscussionModule {}
