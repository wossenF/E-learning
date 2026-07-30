#ifndef HTTP_FRAME_PARSER_H
#define HTTP_FRAME_PARSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <sstream>

namespace temene {

enum class HttpMethod {
  GET, POST, PUT, DELETE, PATCH, OPTIONS, HEAD, UNKNOWN
};

struct HttpRequest {
  HttpMethod method{HttpMethod::UNKNOWN};
  std::string path;
  std::string version;
  std::unordered_map<std::string, std::string> headers;
  std::string body;
  bool complete{false};
};

enum class WebSocketOpcode : uint8_t {
  CONTINUATION = 0x0,
  TEXT = 0x1,
  BINARY = 0x2,
  CLOSE = 0x8,
  PING = 0x9,
  PONG = 0xA
};

struct WebSocketFrame {
  bool fin{true};
  WebSocketOpcode opcode{WebSocketOpcode::TEXT};
  bool masked{false};
  uint64_t payloadLength{0};
  uint8_t maskingKey[4]{0, 0, 0, 0};
  std::vector<uint8_t> payload;
};

class HttpFrameParser {
public:
  static HttpRequest parseHttpRequest(const std::string& rawData);
  static std::string buildHttpResponse(int statusCode, const std::string& statusText, const std::unordered_map<std::string, std::string>& headers, const std::string& body);

  static WebSocketFrame parseWebSocketFrame(const std::vector<uint8_t>& buffer);
  static std::vector<uint8_t> buildWebSocketFrame(WebSocketOpcode opcode, const std::vector<uint8_t>& payload, bool mask = false);

  static std::string calculateWebSocketAcceptKey(const std::string& clientKey);
};

} // namespace temene

#endif // HTTP_FRAME_PARSER_H
