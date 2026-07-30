#include "oauth2_server.h"
#include <random>

namespace temene {

void OAuth2Server::registerClient(const OAuthClient& client) {
  clients_[client.clientId] = client;
}

std::string OAuth2Server::generateRandomString(size_t length) const {
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::mt19937 rng(42); // pseudo-random for testing
  std::uniform_int_distribution<int> dist(0, sizeof(alphanum) - 2);

  std::string s;
  s.reserve(length);
  for (size_t i = 0; i < length; ++i) {
    s += alphanum[dist(rng)];
  }
  return s;
}

std::string OAuth2Server::generateAuthCode(const std::string& clientId, const std::string& redirectUri, const std::string& userId, const std::string& scope) {
  auto it = clients_.find(clientId);
  if (it == clients_.end() || it->second.redirectUri != redirectUri) {
    return "";
  }

  std::string code = generateRandomString(32);
  AuthCode authCode;
  authCode.code = code;
  authCode.clientId = clientId;
  authCode.redirectUri = redirectUri;
  authCode.userId = userId;
  authCode.scope = scope;
  authCode.expiresAt = std::chrono::system_clock::now() + std::chrono::minutes(10);
  
  authCodes_[code] = authCode;
  return code;
}

std::shared_ptr<AccessToken> OAuth2Server::exchangeCodeForToken(const std::string& clientId, const std::string& clientSecret, const std::string& code, const std::string& redirectUri) {
  auto clientIt = clients_.find(clientId);
  if (clientIt == clients_.end() || clientIt->second.clientSecret != clientSecret) return nullptr;

  auto codeIt = authCodes_.find(code);
  if (codeIt == authCodes_.end()) return nullptr;
  
  const AuthCode& authCode = codeIt->second;
  if (authCode.clientId != clientId || authCode.redirectUri != redirectUri) return nullptr;
  if (std::chrono::system_clock::now() > authCode.expiresAt) {
    authCodes_.erase(codeIt);
    return nullptr;
  }

  auto token = std::make_shared<AccessToken>();
  token->token = generateRandomString(64);
  token->refreshToken = generateRandomString(64);
  token->clientId = clientId;
  token->userId = authCode.userId;
  token->scope = authCode.scope;
  token->expiresAt = std::chrono::system_clock::now() + std::chrono::hours(1);

  accessTokens_[token->token] = token;
  refreshTokens_[token->refreshToken] = token->token;
  
  authCodes_.erase(codeIt);
  return token;
}

std::shared_ptr<AccessToken> OAuth2Server::refreshAccessToken(const std::string& clientId, const std::string& clientSecret, const std::string& refreshToken) {
  auto clientIt = clients_.find(clientId);
  if (clientIt == clients_.end() || clientIt->second.clientSecret != clientSecret) return nullptr;

  auto rtIt = refreshTokens_.find(refreshToken);
  if (rtIt == refreshTokens_.end()) return nullptr;

  auto tokenIt = accessTokens_.find(rtIt->second);
  if (tokenIt == accessTokens_.end()) return nullptr;
  
  auto oldToken = tokenIt->second;
  if (oldToken->clientId != clientId) return nullptr;

  auto newToken = std::make_shared<AccessToken>();
  newToken->token = generateRandomString(64);
  newToken->refreshToken = generateRandomString(64);
  newToken->clientId = clientId;
  newToken->userId = oldToken->userId;
  newToken->scope = oldToken->scope;
  newToken->expiresAt = std::chrono::system_clock::now() + std::chrono::hours(1);

  accessTokens_.erase(tokenIt);
  refreshTokens_.erase(rtIt);

  accessTokens_[newToken->token] = newToken;
  refreshTokens_[newToken->refreshToken] = newToken->token;
  
  return newToken;
}

bool OAuth2Server::validateToken(const std::string& token, std::string& outUserId, std::string& outScope) {
  auto it = accessTokens_.find(token);
  if (it == accessTokens_.end()) return false;
  
  if (std::chrono::system_clock::now() > it->second->expiresAt) {
    refreshTokens_.erase(it->second->refreshToken);
    accessTokens_.erase(it);
    return false;
  }

  outUserId = it->second->userId;
  outScope = it->second->scope;
  return true;
}

} // namespace temene
