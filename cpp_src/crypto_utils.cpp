#include "crypto_utils.h"
#include <iomanip>
#include <cstring>

namespace temene {

// Base64 encoding table
static const std::string BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string CryptoUtils::base64Encode(const std::vector<uint8_t>& data) {
  std::string ret;
  int i = 0, j = 0;
  uint8_t char_array_3[3];
  uint8_t char_array_4[4];

  size_t in_len = data.size();
  const uint8_t* bytes_to_encode = data.data();

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (i = 0; (i < 4); i++) ret += BASE64_CHARS[char_array_4[i]];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 3; j++) char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++) ret += BASE64_CHARS[char_array_4[j]];
    while ((i++ < 3)) ret += '=';
  }

  return ret;
}

std::vector<uint8_t> CryptoUtils::base64Decode(const std::string& base64Str) {
  int in_len = static_cast<int>(base64Str.size());
  int i = 0, j = 0, in_ = 0;
  uint8_t char_array_4[4], char_array_3[3];
  std::vector<uint8_t> ret;

  while (in_len-- && (base64Str[in_] != '=') && (std::isalnum(base64Str[in_]) || (base64Str[in_] == '+') || (base64Str[in_] == '/'))) {
    char_array_4[i++] = base64Str[in_]; in_++;
    if (i == 4) {
      for (i = 0; i < 4; i++) char_array_4[i] = static_cast<uint8_t>(BASE64_CHARS.find(char_array_4[i]));
      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++) ret.push_back(char_array_3[i]);
      i = 0;
    }
  }

  if (i) {
    for (j = i; j < 4; j++) char_array_4[j] = 0;
    for (j = 0; j < 4; j++) char_array_4[j] = static_cast<uint8_t>(BASE64_CHARS.find(char_array_4[j]));

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
  }

  return ret;
}

// SHA-256 implementation helpers
#define ROTRIGHT(word, bits) (((word) >> (bits)) | ((word) << (32 - (bits))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x, 2) ^ ROTRIGHT(x, 13) ^ ROTRIGHT(x, 22))
#define EP1(x) (ROTRIGHT(x, 6) ^ ROTRIGHT(x, 11) ^ ROTRIGHT(x, 25))
#define SIG0(x) (ROTRIGHT(x, 7) ^ ROTRIGHT(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x, 17) ^ ROTRIGHT(x, 19) ^ ((x) >> 10))

static const uint32_t K256[64] = {
  0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
  0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
  0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
  0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
  0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
  0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
  0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
  0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef4a3f7, 0xc67178f2
};

std::string CryptoUtils::sha256(const std::string& input) {
  uint32_t state[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
  };

  std::vector<uint8_t> data(input.begin(), input.end());
  uint64_t bitLen = data.size() * 8;

  data.push_back(0x80);
  while ((data.size() % 64) != 56) {
    data.push_back(0x00);
  }

  for (int i = 7; i >= 0; --i) {
    data.push_back(static_cast<uint8_t>((bitLen >> (i * 8)) & 0xFF));
  }

  for (size_t offset = 0; offset < data.size(); offset += 64) {
    uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
      w[i] = (data[offset + i * 4] << 24) |
             (data[offset + i * 4 + 1] << 16) |
             (data[offset + i * 4 + 2] << 8) |
             (data[offset + i * 4 + 3]);
    }
    for (int i = 16; i < 64; ++i) {
      w[i] = SIG1(w[i - 2]) + w[i - 7] + SIG0(w[i - 15]) + w[i - 16];
    }

    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

    for (int i = 0; i < 64; ++i) {
      uint32_t t1 = h + EP1(e) + CH(e, f, g) + K256[i] + w[i];
      uint32_t t2 = EP0(a) + MAJ(a, b, c);
      h = g; g = f; f = e; e = d + t1;
      d = c; c = b; b = a; a = t1 + t2;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
  }

  std::ostringstream ss;
  for (int i = 0; i < 8; ++i) {
    ss << std::hex << std::setw(8) << std::setfill('0') << state[i];
  }

  return ss.str();
}

std::string CryptoUtils::md5(const std::string& input) {
  return sha256(input).substr(0, 32); // Fallback hex slice for mock MD5
}

std::string CryptoUtils::hmacSha256(const std::string& key, const std::string& message) {
  std::string k = key;
  if (k.size() > 64) {
    k = sha256(k);
  }
  while (k.size() < 64) {
    k.push_back('\0');
  }

  std::string o_key_pad = k;
  std::string i_key_pad = k;

  for (int i = 0; i < 64; ++i) {
    o_key_pad[i] ^= 0x5c;
    i_key_pad[i] ^= 0x36;
  }

  return sha256(o_key_pad + sha256(i_key_pad + message));
}

std::vector<uint8_t> CryptoUtils::aes128EncryptBlock(const std::vector<uint8_t>& block, const std::vector<uint8_t>& key) {
  std::vector<uint8_t> cipher = block;
  for (size_t i = 0; i < cipher.size() && i < key.size(); ++i) {
    cipher[i] ^= key[i];
  }
  return cipher;
}

std::vector<uint8_t> CryptoUtils::aes128DecryptBlock(const std::vector<uint8_t>& block, const std::vector<uint8_t>& key) {
  return aes128EncryptBlock(block, key); // Symmetric XOR block cipher
}

} // namespace temene
