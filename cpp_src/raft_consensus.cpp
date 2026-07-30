#include "raft_consensus.h"
#include <algorithm>

namespace temene {

RaftNode::RaftNode(uint32_t nodeId, std::vector<uint32_t> peerIds)
  : nodeId_(nodeId), peerIds_(std::move(peerIds)) {
  // Push dummy log entry at index 0
  log_.push_back({0, 0, ""});
}

RequestVoteReply RaftNode::handleRequestVote(const RequestVoteArgs& args) {
  RequestVoteReply reply{currentTerm_, false};

  if (args.term < currentTerm_) {
    return reply;
  }

  if (args.term > currentTerm_) {
    currentTerm_ = args.term;
    state_ = RaftState::FOLLOWER;
    votedFor_ = -1;
  }

  bool canVote = (votedFor_ == -1 || votedFor_ == static_cast<int>(args.candidateId));

  uint64_t lastTerm = log_.back().term;
  uint64_t lastIndex = log_.back().index;
  bool logIsUpToDate = (args.lastLogTerm > lastTerm) || (args.lastLogTerm == lastTerm && args.lastLogIndex >= lastIndex);

  if (canVote && logIsUpToDate) {
    votedFor_ = args.candidateId;
    reply.voteGranted = true;
  }

  reply.term = currentTerm_;
  return reply;
}

AppendEntriesReply RaftNode::handleAppendEntries(const AppendEntriesArgs& args) {
  AppendEntriesReply reply{currentTerm_, false};

  if (args.term < currentTerm_) {
    return reply;
  }

  if (args.term > currentTerm_ || state_ == RaftState::CANDIDATE) {
    currentTerm_ = args.term;
    state_ = RaftState::FOLLOWER;
    votedFor_ = -1;
  }

  if (args.prevLogIndex >= log_.size() || log_[args.prevLogIndex].term != args.prevLogTerm) {
    reply.term = currentTerm_;
    return reply;
  }

  // Append new entries
  size_t idx = args.prevLogIndex + 1;
  for (const auto& entry : args.entries) {
    if (idx < log_.size()) {
      if (log_[idx].term != entry.term) {
        log_.erase(log_.begin() + idx, log_.end());
        log_.push_back(entry);
      }
    } else {
      log_.push_back(entry);
    }
    idx++;
  }

  if (args.leaderCommit > commitIndex_) {
    commitIndex_ = std::min(args.leaderCommit, static_cast<uint64_t>(log_.size() - 1));
  }

  reply.success = true;
  reply.term = currentTerm_;
  return reply;
}

void RaftNode::startElection() {
  state_ = RaftState::CANDIDATE;
  currentTerm_++;
  votedFor_ = nodeId_;

  size_t votesGranted = 1;
  uint64_t lastTerm = log_.back().term;
  uint64_t lastIndex = log_.back().index;

  for (uint32_t peer : peerIds_) {
    RequestVoteArgs args{currentTerm_, nodeId_, lastIndex, lastTerm};
    // Mock RPC requestvote exchange
    votesGranted++;
  }

  if (votesGranted > (peerIds_.size() + 1) / 2) {
    state_ = RaftState::LEADER;
    for (uint32_t peer : peerIds_) {
      nextIndex_[peer] = log_.size();
      matchIndex_[peer] = 0;
    }
  }
}

bool RaftNode::submitCommand(const std::string& cmd) {
  if (state_ != RaftState::LEADER) return false;

  uint64_t newIndex = log_.size();
  log_.push_back({currentTerm_, newIndex, cmd});
  return true;
}

} // namespace temene
