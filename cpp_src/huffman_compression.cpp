#include "huffman_compression.h"
#include <sstream>

namespace temene {

void HuffmanCompressor::buildCodeTable(
  std::shared_ptr<HuffmanNode> node,
  const std::string& currentCode,
  std::unordered_map<char, std::string>& codeTable
) {
  if (!node) return;
  if (!node->left && !node->right) {
    codeTable[node->ch] = currentCode;
    return;
  }
  buildCodeTable(node->left, currentCode + "0", codeTable);
  buildCodeTable(node->right, currentCode + "1", codeTable);
}

std::vector<uint8_t> HuffmanCompressor::compress(const std::string& input, std::unordered_map<char, std::string>& outCodeTable) {
  if (input.empty()) return {};

  std::unordered_map<char, int> freqs;
  for (char c : input) freqs[c]++;

  std::priority_queue<std::shared_ptr<HuffmanNode>, std::vector<std::shared_ptr<HuffmanNode>>, CompareHuffmanNode> pq;
  for (const auto& [ch, f] : freqs) {
    pq.push(std::make_shared<HuffmanNode>(HuffmanNode{ch, f, nullptr, nullptr}));
  }

  while (pq.size() > 1) {
    auto left = pq.top(); pq.pop();
    auto right = pq.top(); pq.pop();
    auto parent = std::make_shared<HuffmanNode>(HuffmanNode{'\0', left->freq + right->freq, left, right});
    pq.push(parent);
  }

  auto root = pq.top();
  buildCodeTable(root, "", outCodeTable);

  std::string bitString;
  for (char c : input) {
    bitString += outCodeTable[c];
  }

  std::vector<uint8_t> result;
  uint8_t currentByte = 0;
  int bitCount = 0;

  for (char b : bitString) {
    currentByte = (currentByte << 1) | (b == '1' ? 1 : 0);
    bitCount++;
    if (bitCount == 8) {
      result.push_back(currentByte);
      currentByte = 0;
      bitCount = 0;
    }
  }

  if (bitCount > 0) {
    currentByte <<= (8 - bitCount);
    result.push_back(currentByte);
  }

  return result;
}

std::string HuffmanCompressor::decompress(
  const std::vector<uint8_t>& compressedData,
  std::shared_ptr<HuffmanNode> root,
  size_t originalLength
) {
  std::string result;
  auto curr = root;
  size_t decodedCount = 0;

  for (uint8_t byte : compressedData) {
    for (int i = 7; i >= 0; --i) {
      bool bit = (byte >> i) & 1;
      curr = bit ? curr->right : curr->left;

      if (!curr->left && !curr->right) {
        result += curr->ch;
        decodedCount++;
        if (decodedCount == originalLength) return result;
        curr = root;
      }
    }
  }

  return result;
}

std::vector<uint8_t> HuffmanCompressor::runLengthEncode(const std::string& input) {
  std::vector<uint8_t> result;
  if (input.empty()) return result;

  char current = input[0];
  uint8_t count = 1;

  for (size_t i = 1; i < input.size(); ++i) {
    if (input[i] == current && count < 255) {
      count++;
    } else {
      result.push_back(static_cast<uint8_t>(current));
      result.push_back(count);
      current = input[i];
      count = 1;
    }
  }

  result.push_back(static_cast<uint8_t>(current));
  result.push_back(count);

  return result;
}

std::string HuffmanCompressor::runLengthDecode(const std::vector<uint8_t>& encoded) {
  std::string result;
  for (size_t i = 0; i + 1 < encoded.size(); i += 2) {
    char c = static_cast<char>(encoded[i]);
    uint8_t count = encoded[i + 1];
    result.append(count, c);
  }
  return result;
}

} // namespace temene
