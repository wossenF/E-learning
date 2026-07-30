#ifndef HUFFMAN_COMPRESSION_H
#define HUFFMAN_COMPRESSION_H

#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <memory>
#include <cstdint>

namespace temene {

struct HuffmanNode {
  char ch;
  int freq;
  std::shared_ptr<HuffmanNode> left{nullptr};
  std::shared_ptr<HuffmanNode> right{nullptr};
};

struct CompareHuffmanNode {
  bool operator()(const std::shared_ptr<HuffmanNode>& a, const std::shared_ptr<HuffmanNode>& b) const {
    return a->freq > b->freq;
  }
};

class HuffmanCompressor {
public:
  std::vector<uint8_t> compress(const std::string& input, std::unordered_map<char, std::string>& outCodeTable);
  std::string decompress(const std::vector<uint8_t>& compressedData, std::shared_ptr<HuffmanNode> root, size_t originalLength);

  static std::vector<uint8_t> runLengthEncode(const std::string& input);
  static std::string runLengthDecode(const std::vector<uint8_t>& encoded);

private:
  void buildCodeTable(std::shared_ptr<HuffmanNode> node, const std::string& currentCode, std::unordered_map<char, std::string>& codeTable);
};

} // namespace temene

#endif // HUFFMAN_COMPRESSION_H
