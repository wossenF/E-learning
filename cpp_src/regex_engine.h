#ifndef REGEX_ENGINE_H
#define REGEX_ENGINE_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>
#include <iostream>

namespace temene {

struct NfaState {
  int id;
  char matchChar{'\0'}; // '\0' for epsilon transition
  std::shared_ptr<NfaState> out1{nullptr};
  std::shared_ptr<NfaState> out2{nullptr};
  bool isMatch{false};
};

class RegexEngine {
public:
  bool matchPattern(const std::string& pattern, const std::string& text);

private:
  std::shared_ptr<NfaState> buildNfa(const std::string& pattern);
  void addState(std::shared_ptr<NfaState> state, std::vector<std::shared_ptr<NfaState>>& currentStates, std::unordered_set<int>& visited);
};

} // namespace temene

#endif // REGEX_ENGINE_H
