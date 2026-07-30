#ifndef TEXT_SEARCH_ENGINE_H
#define TEXT_SEARCH_ENGINE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace temene {

struct SearchResult {
  int docId;
  double score;
  std::string snippet;
};

class PorterStemmer {
public:
  static std::string stem(const std::string& word);
};

class TextSearchEngine {
public:
  void addDocument(int docId, const std::string& text);

  std::vector<SearchResult> searchBM25(const std::string& query, double k1 = 1.5, double b = 0.75) const;
  double calculateJaccardTrigramSimilarity(const std::string& str1, const std::string& str2) const;

  size_t totalDocuments() const { return documentStore_.size(); }

private:
  std::unordered_map<int, std::string> documentStore_;
  std::unordered_map<int, std::vector<std::string>> docTokens_;
  std::unordered_map<int, size_t> docLengths_;
  std::unordered_map<std::string, std::unordered_map<int, int>> invertedIndex_; // term -> { docId -> tf }

  double avgDocLength_{0.0};

  std::vector<std::string> tokenizeAndStem(const std::string& text) const;
  std::vector<std::string> extractTrigrams(const std::string& text) const;
};

} // namespace temene

#endif // TEXT_SEARCH_ENGINE_H
