#ifndef CRYPTOGRAPHY_ENGINE_V2_H
#define CRYPTOGRAPHY_ENGINE_V2_H

#include <vector>
#include <string>
#include <cstdint>
#include <array>
#include <memory>
#include <iostream>

namespace temene {

struct KeyPairEd25519 {
  std::vector<uint8_t> publicKey;
  std::vector<uint8_t> privateKey;
};

struct KeyPairRsa {
  std::vector<uint8_t> n; // modulus
  std::vector<uint8_t> e; // public exponent
  std::vector<uint8_t> d; // private exponent
  std::vector<uint8_t> p; // prime 1
  std::vector<uint8_t> q; // prime 2
  std::vector<uint8_t> dp; // d mod (p-1)
  std::vector<uint8_t> dq; // d mod (q-1)
  std::vector<uint8_t> qInv; // q^-1 mod p
};

class CryptographyEngineV2 {
public:
  static KeyPairEd25519 generateEd25519KeyPair();
  static std::vector<uint8_t> signEd25519(const std::vector<uint8_t>& message, const KeyPairEd25519& keyPair);
  static bool verifyEd25519(const std::vector<uint8_t>& message, const std::vector<uint8_t>& signature, const std::vector<uint8_t>& publicKey);

  static KeyPairRsa generateRsaKeyPair(int bits = 2048);
  static std::vector<uint8_t> rsaEncrypt(const std::vector<uint8_t>& message, const std::vector<uint8_t>& n, const std::vector<uint8_t>& e);
  static std::vector<uint8_t> rsaDecrypt(const std::vector<uint8_t>& ciphertext, const KeyPairRsa& keyPair);
  static std::vector<uint8_t> rsaSign(const std::vector<uint8_t>& hash, const KeyPairRsa& keyPair);
  static bool rsaVerify(const std::vector<uint8_t>& hash, const std::vector<uint8_t>& signature, const std::vector<uint8_t>& n, const std::vector<uint8_t>& e);

  static std::vector<uint8_t> encryptAes256Gcm(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv, const std::vector<uint8_t>& aad, std::vector<uint8_t>& outTag);
  static std::vector<uint8_t> decryptAes256Gcm(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv, const std::vector<uint8_t>& aad, const std::vector<uint8_t>& tag, bool& outValid);

  static std::vector<uint8_t> encryptChaCha20Poly1305(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& key, const std::vector<uint8_t>& nonce, const std::vector<uint8_t>& aad, std::vector<uint8_t>& outTag);
  static std::vector<uint8_t> decryptChaCha20Poly1305(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& key, const std::vector<uint8_t>& nonce, const std::vector<uint8_t>& aad, const std::vector<uint8_t>& tag, bool& outValid);

  static std::vector<uint8_t> hkdfExtract(const std::vector<uint8_t>& salt, const std::vector<uint8_t>& ikm);
  static std::vector<uint8_t> hkdfExpand(const std::vector<uint8_t>& prk, const std::vector<uint8_t>& info, size_t length);

private:
  // Internal helper methods for RSA math
  static std::vector<uint8_t> bigIntAdd(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b);
  static std::vector<uint8_t> bigIntSub(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b);
  static std::vector<uint8_t> bigIntMul(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b);
  static std::vector<uint8_t> bigIntDiv(const std::vector<uint8_t>& a, const std::vector<uint8_t>& b, std::vector<uint8_t>& outRemainder);
  static std::vector<uint8_t> bigIntModExp(const std::vector<uint8_t>& base, const std::vector<uint8_t>& exp, const std::vector<uint8_t>& mod);
  static std::vector<uint8_t> generateLargePrime(int bits);
  static bool millerRabinTest(const std::vector<uint8_t>& n, int k);

  // Internal helper for AES
  static void ghash(const std::vector<uint8_t>& h, const std::vector<uint8_t>& data, std::vector<uint8_t>& y);
  
  // Internal helper for Poly1305
  static void poly1305Update(std::vector<uint8_t>& acc, const std::vector<uint8_t>& block, const std::vector<uint8_t>& r);
};

} // namespace temene

#endif // CRYPTOGRAPHY_ENGINE_V2_H
