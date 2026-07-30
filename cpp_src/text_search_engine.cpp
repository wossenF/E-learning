#include "text_search_engine.h"
#include <cctype>
#include <sstream>

namespace temene {

std::string PorterStemmer::stem(const std::string& word) {
  std::string w = word;
  std::transform(w.begin(), w.end(), w.begin(), ::tolower);

  if (w.size() > 4) {
    if (w.rfind("ing") == w.size() - 3) w = w.substr(0, w.size() - 3);
    else if (w.rfind("ies") == w.size() - 3) w = w.substr(0, w.size() - 3) + "y";
    else if (w.rfind("es") == w.size() - 2) w = w.substr(0, w.size() - 2);
    else if (w.rfind("s") == w.size() - 1) w = w.substr(0, w.size() - 1);
    else if (w.rfind("ed") == w.size() - 2) w = w.substr(0, w.size() - 2);
    else if (w.rfind("ly") == w.size() - 2) w = w.substr(0, w.size() - 2);
  }

  return w;
}

std::vector<std::string> TextSearchEngine::tokenizeAndStem(const std::string& text) const {
  std::vector<std::string> tokens;
  std::string clean;

  for (char c : text) {
    if (std::isalnum(c)) clean += c;
    else clean += ' ';
  }

  std::istringstream stream(clean);
  std::string word;
  while (stream >> word) {
    if (word.size() > 1) {
      tokens.push_back(PorterStemmer::stem(word));
    }
  }

  return tokens;
}

std::vector<std::string> TextSearchEngine::extractTrigrams(const std::string& text) const {
  std::vector<std::string> trigrams;
  if (text.size() < 3) return trigrams;
  for (size_t i = 0; i <= text.size() - 3; ++i) {
    trigrams.push_back(text.substr(i, 3));
  }
  return trigrams;
}

void TextSearchEngine::addDocument(int docId, const std::string& text) {
  documentStore_[docId] = text;
  auto tokens = tokenizeAndStem(text);
  docTokens_[docId] = tokens;
  docLengths_[docId] = tokens.size();

  std::unordered_map<std::string, int> tfMap;
  for (const auto& t : tokens) {
    tfMap[t]++;
  }

  for (const auto& [term, tf] : tfMap) {
    invertedIndex_[term][docId] = tf;
  }

  // Update avg length
  size_t totalLen = 0;
  for (const auto& [_, len] : docLengths_) totalLen += len;
  avgDocLength_ = static_cast<double>(totalLen) / docLengths_.size();
}

std::vector<SearchResult> TextSearchEngine::searchBM25(const std::string& query, double k1, double b) const {
  auto queryTokens = tokenizeAndStem(query);
  std::unordered_map<int, double> docScores;

  double N = static_cast<double>(documentStore_.size());
  if (N == 0.0) return {};

  for (const auto& term : queryTokens) {
    auto invIt = invertedIndex_.find(term);
    if (invIt == invertedIndex_.end()) continue;

    const auto& postings = invIt->second;
    double n_q = static_cast<double>(postings.size());
    double idf = std::log((N - n_q + 0.5) / (n_q + 0.5) + 1.0);

    for (const auto& [docId, tf] : postings) {
      double docLen = static_cast<double>(docLengths_.at(docId));
      double num = tf * (k1 + 1.0);
      double den = tf + k1 * (1.0 - b + b * (docLen / (avgDocLength_ > 0 ? avgDocLength_ : 1.0)));
      docScores[docId] += idf * (num / den);
    }
  }

  std::vector<SearchResult> results;
  for (const auto& [docId, score] : docScores) {
    std::string fullText = documentStore_.at(docId);
    std::string snippet = fullText.size() > 100 ? fullText.substr(0, 100) + "..." : fullText;
    results.push_back({docId, score, snippet});
  }

  std::sort(results.begin(), results.end(), [](const SearchResult& a, const SearchResult& b) {
    return a.score > b.score;
  });

  return results;
}

double TextSearchEngine::calculateJaccardTrigramSimilarity(const std::string& str1, const std::string& str2) const {
  auto tri1 = extractTrigrams(str1);
  auto tri2 = extractTrigrams(str2);

  std::unordered_set<std::string> set1(tri1.begin(), tri1.end());
  std::unordered_set<std::string> set2(tri2.begin(), tri2.end());

  if (set1.empty() && set2.empty()) return 1.0;
  if (set1.empty() || set2.empty()) return 0.0;

  size_t intersectionCount = 0;
  for (const auto& t : set1) {
    if (set2.count(t)) intersectionCount++;
  }

  size_t unionCount = set1.size() + set2.size() - intersectionCount;
  return static_cast<double>(intersectionCount) / unionCount;
}

} // namespace temene
