#ifndef RAFT_CONSENSUS_H
#define RAFT_CONSENSUS_H

#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

namespace temene {

enum class RaftState { FOLLOWER, CANDIDATE, LEADER };

struct LogEntry {
  uint64_t term;
  uint64_t index;
  std::string command;
};

struct RequestVoteArgs {
  uint64_t term;
  uint32_t candidateId;
  uint64_t lastLogIndex;
  uint64_t lastLogTerm;
};

struct RequestVoteReply {
  uint64_t term;
  bool voteGranted;
};

struct AppendEntriesArgs {
  uint64_t term;
  uint32_t leaderId;
  uint64_t prevLogIndex;
  uint64_t prevLogTerm;
  std::vector<LogEntry> entries;
  uint64_t leaderCommit;
};

struct AppendEntriesReply {
  uint64_t term;
  bool success;
};

class RaftNode {
public:
  explicit RaftNode(uint32_t nodeId, std::vector<uint32_t> peerIds);

  RequestVoteReply handleRequestVote(const RequestVoteArgs& args);
  AppendEntriesReply handleAppendEntries(const AppendEntriesArgs& args);

  void startElection();
  bool submitCommand(const std::string& cmd);

  RaftState getState() const { return state_; }
  uint64_t getCurrentTerm() const { return currentTerm_; }
  size_t getLogSize() const { return log_.size(); }

private:
  uint32_t nodeId_;
  std::vector<uint32_t> peerIds_;

  RaftState state_{RaftState::FOLLOWER};
  uint64_t currentTerm_{0};
  int votedFor_{-1};
  std::vector<LogEntry> log_;

  uint64_t commitIndex_{0};
  uint64_t lastApplied_{0};

  std::unordered_map<uint32_t, uint64_t> nextIndex_;
  std::unordered_map<uint32_t, uint64_t> matchIndex_;
};

} // namespace temene

#endif // RAFT_CONSENSUS_H
