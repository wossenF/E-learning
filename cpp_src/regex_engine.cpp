#include "regex_engine.h"

namespace temene {

std::shared_ptr<NfaState> RegexEngine::buildNfa(const std::string& pattern) {
  int stateId = 0;
  auto start = std::make_shared<NfaState>(NfaState{stateId++, '\0', nullptr, nullptr, false});
  auto curr = start;

  for (size_t i = 0; i < pattern.size(); ++i) {
    char c = pattern[i];
    if (i + 1 < pattern.size() && pattern[i + 1] == '*') {
      auto loop = std::make_shared<NfaState>(NfaState{stateId++, c, nullptr, nullptr, false});
      auto next = std::make_shared<NfaState>(NfaState{stateId++, '\0', nullptr, nullptr, false});

      curr->out1 = loop;
      curr->out2 = next;
      loop->out1 = loop;
      loop->out2 = next;
      curr = next;
      i++; // Skip '*'
    } else {
      auto next = std::make_shared<NfaState>(NfaState{stateId++, c, nullptr, nullptr, false});
      curr->out1 = next;
      curr = next;
    }
  }

  curr->isMatch = true;
  return start;
}

void RegexEngine::addState(
  std::shared_ptr<NfaState> state,
  std::vector<std::shared_ptr<NfaState>>& currentStates,
  std::unordered_set<int>& visited
) {
  if (!state || visited.count(state->id)) return;
  visited.insert(state->id);

  if (state->matchChar == '\0' && !state->isMatch) {
    addState(state->out1, currentStates, visited);
    addState(state->out2, currentStates, visited);
  } else {
    currentStates.push_back(state);
  }
}

bool RegexEngine::matchPattern(const std::string& pattern, const std::string& text) {
  auto start = buildNfa(pattern);
  std::vector<std::shared_ptr<NfaState>> currentStates;
  std::unordered_set<int> visited;

  addState(start, currentStates, visited);

  for (char c : text) {
    std::vector<std::shared_ptr<NfaState>> nextStates;
    visited.clear();

    for (const auto& state : currentStates) {
      if (state->matchChar == c || state->matchChar == '.') {
        addState(state->out1, nextStates, visited);
      }
    }
    currentStates = nextStates;
  }

  for (const auto& state : currentStates) {
    if (state->isMatch) return true;
  }

  return false;
}

} // namespace temene
