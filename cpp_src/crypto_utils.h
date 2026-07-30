#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <string>
#include <vector>
#include <cstdint>
#include <sstream>

namespace temene {

class CryptoUtils {
public:
  static std::string sha256(const std::string& input);
  static std::string md5(const std::string& input);
  static std::string hmacSha256(const std::string& key, const std::string& message);

  static std::string base64Encode(const std::vector<uint8_t>& data);
  static std::vector<uint8_t> base64Decode(const std::string& base64Str);

  static std::vector<uint8_t> aes128EncryptBlock(const std::vector<uint8_t>& block, const std::vector<uint8_t>& key);
  static std::vector<uint8_t> aes128DecryptBlock(const std::vector<uint8_t>& block, const std::vector<uint8_t>& key);
};

} // namespace temene

#endif // CRYPTO_UTILS_H
