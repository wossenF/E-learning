#include "http_frame_parser.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <stdexcept>

namespace temene {

static std::string trim(const std::string& str) {
  size_t first = str.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return "";
  size_t last = str.find_last_not_of(" \t\r\n");
  return str.substr(first, (last - first + 1));
}

HttpMethod parseMethodString(const std::string& m) {
  if (m == "GET") return HttpMethod::GET;
  if (m == "POST") return HttpMethod::POST;
  if (m == "PUT") return HttpMethod::PUT;
  if (m == "DELETE") return HttpMethod::DELETE;
  if (m == "PATCH") return HttpMethod::PATCH;
  if (m == "OPTIONS") return HttpMethod::OPTIONS;
  if (m == "HEAD") return HttpMethod::HEAD;
  return HttpMethod::UNKNOWN;
}

HttpRequest HttpFrameParser::parseHttpRequest(const std::string& rawData) {
  HttpRequest req;
  size_t headerEnd = rawData.find("\r\n\r\n");
  if (headerEnd == std::string::npos) {
    headerEnd = rawData.find("\n\n");
    if (headerEnd == std::string::npos) return req;
  }

  std::string headerPart = rawData.substr(0, headerEnd);
  std::istringstream stream(headerPart);
  std::string requestLine;

  if (std::getline(stream, requestLine)) {
    requestLine = trim(requestLine);
    std::istringstream lineStream(requestLine);
    std::string methodStr, pathStr, versionStr;
    lineStream >> methodStr >> pathStr >> versionStr;

    req.method = parseMethodString(methodStr);
    req.path = pathStr;
    req.version = versionStr;
  }

  std::string line;
  while (std::getline(stream, line)) {
    line = trim(line);
    if (line.empty()) continue;

    size_t colon = line.find(':');
    if (colon != std::string::npos) {
      std::string name = trim(line.substr(0, colon));
      std::string value = trim(line.substr(colon + 1));

      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      req.headers[name] = value;
    }
  }

  size_t bodyStart = headerEnd + (rawData.find("\r\n\r\n") != std::string::npos ? 4 : 2);
  if (bodyStart < rawData.size()) {
    req.body = rawData.substr(bodyStart);
  }

  req.complete = true;
  return req;
}

std::string HttpFrameParser::buildHttpResponse(
  int statusCode,
  const std::string& statusText,
  const std::unordered_map<std::string, std::string>& headers,
  const std::string& body
) {
  std::ostringstream ss;
  ss << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
  ss << "Content-Length: " << body.size() << "\r\n";

  for (const auto& [k, v] : headers) {
    ss << k << ": " << v << "\r\n";
  }

  ss << "\r\n" << body;
  return ss.str();
}

WebSocketFrame HttpFrameParser::parseWebSocketFrame(const std::vector<uint8_t>& buffer) {
  if (buffer.size() < 2) throw std::runtime_error("WebSocket buffer too small for header.");

  WebSocketFrame frame;
  frame.fin = (buffer[0] & 0x80) != 0;
  frame.opcode = static_cast<WebSocketOpcode>(buffer[0] & 0x0F);

  frame.masked = (buffer[1] & 0x80) != 0;
  uint8_t len7 = buffer[1] & 0x7F;

  size_t offset = 2;
  if (len7 == 126) {
    if (buffer.size() < 4) throw std::runtime_error("WebSocket 16-bit length truncated.");
    frame.payloadLength = (static_cast<uint64_t>(buffer[2]) << 8) | buffer[3];
    offset = 4;
  } else if (len7 == 127) {
    if (buffer.size() < 10) throw std::runtime_error("WebSocket 64-bit length truncated.");
    uint64_t l = 0;
    for (int i = 0; i < 8; ++i) {
      l = (l << 8) | buffer[2 + i];
    }
    frame.payloadLength = l;
    offset = 10;
  } else {
    frame.payloadLength = len7;
  }

  if (frame.masked) {
    if (buffer.size() < offset + 4) throw std::runtime_error("WebSocket masking key missing.");
    std::memcpy(frame.maskingKey, &buffer[offset], 4);
    offset += 4;
  }

  if (buffer.size() < offset + frame.payloadLength) {
    throw std::runtime_error("WebSocket payload truncated.");
  }

  frame.payload.assign(buffer.begin() + offset, buffer.begin() + offset + frame.payloadLength);

  if (frame.masked) {
    for (size_t i = 0; i < frame.payload.size(); ++i) {
      frame.payload[i] ^= frame.maskingKey[i % 4];
    }
  }

  return frame;
}

std::vector<uint8_t> HttpFrameParser::buildWebSocketFrame(WebSocketOpcode opcode, const std::vector<uint8_t>& payload, bool mask) {
  std::vector<uint8_t> frame;

  uint8_t b0 = 0x80 | (static_cast<uint8_t>(opcode) & 0x0F);
  frame.push_back(b0);

  uint8_t maskBit = mask ? 0x80 : 0x00;
  size_t len = payload.size();

  if (len <= 125) {
    frame.push_back(maskBit | static_cast<uint8_t>(len));
  } else if (len <= 65535) {
    frame.push_back(maskBit | 126);
    frame.push_back(static_cast<uint8_t>((len >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(len & 0xFF));
  } else {
    frame.push_back(maskBit | 127);
    for (int i = 7; i >= 0; --i) {
      frame.push_back(static_cast<uint8_t>((len >> (i * 8)) & 0xFF));
    }
  }

  if (mask) {
    uint8_t mkey[4] = {0x12, 0x34, 0x56, 0x78};
    frame.insert(frame.end(), mkey, mkey + 4);
    for (size_t i = 0; i < payload.size(); ++i) {
      frame.push_back(payload[i] ^ mkey[i % 4]);
    }
  } else {
    frame.insert(frame.end(), payload.begin(), payload.end());
  }

  return frame;
}

std::string HttpFrameParser::calculateWebSocketAcceptKey(const std::string& clientKey) {
  return clientKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
}

} // namespace temene
