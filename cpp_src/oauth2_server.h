#ifndef OAUTH2_SERVER_H
#define OAUTH2_SERVER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>

namespace temene {

struct OAuthClient {
  std::string clientId;
  std::string clientSecret;
  std::string redirectUri;
  std::vector<std::string> allowedGrants;
};

struct AuthCode {
  std::string code;
  std::string clientId;
  std::string redirectUri;
  std::string userId;
  std::string scope;
  std::chrono::system_clock::time_point expiresAt;
};

struct AccessToken {
  std::string token;
  std::string refreshToken;
  std::string clientId;
  std::string userId;
  std::string scope;
  std::chrono::system_clock::time_point expiresAt;
};

class OAuth2Server {
public:
  void registerClient(const OAuthClient& client);
  
  std::string generateAuthCode(const std::string& clientId, const std::string& redirectUri, const std::string& userId, const std::string& scope);
  
  std::shared_ptr<AccessToken> exchangeCodeForToken(const std::string& clientId, const std::string& clientSecret, const std::string& code, const std::string& redirectUri);
  
  std::shared_ptr<AccessToken> refreshAccessToken(const std::string& clientId, const std::string& clientSecret, const std::string& refreshToken);

  bool validateToken(const std::string& token, std::string& outUserId, std::string& outScope);

private:
  std::unordered_map<std::string, OAuthClient> clients_;
  std::unordered_map<std::string, AuthCode> authCodes_;
  std::unordered_map<std::string, std::shared_ptr<AccessToken>> accessTokens_;
  std::unordered_map<std::string, std::string> refreshTokens_; // refreshToken -> accessToken

  std::string generateRandomString(size_t length) const;
};

} // namespace temene

#endif // OAUTH2_SERVER_H
