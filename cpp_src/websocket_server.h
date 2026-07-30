#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

namespace temene {

enum class WsOpcode : uint8_t {
  CONTINUATION = 0x0,
  TEXT = 0x1,
  BINARY = 0x2,
  CLOSE = 0x8,
  PING = 0x9,
  PONG = 0xA
};

struct WsFrame {
  bool fin;
  WsOpcode opcode;
  bool masked;
  std::vector<uint8_t> maskingKey;
  std::vector<uint8_t> payload;
};

using WsConnectionId = uint32_t;
using WsMessageHandler = std::function<void(WsConnectionId, const std::vector<uint8_t>&, bool isText)>;
using WsCloseHandler = std::function<void(WsConnectionId)>;

class WebSocketServer {
public:
  WebSocketServer();

  void setMessageHandler(WsMessageHandler handler);
  void setCloseHandler(WsCloseHandler handler);

  void handleIncomingData(WsConnectionId connId, const std::vector<uint8_t>& data);
  std::vector<uint8_t> buildFrame(const std::vector<uint8_t>& payload, WsOpcode opcode, bool fin = true) const;
  
  std::vector<uint8_t> generateAcceptKey(const std::string& clientKey) const;

private:
  WsMessageHandler onMessage_;
  WsCloseHandler onClose_;

  struct ConnectionState {
    std::vector<uint8_t> buffer;
  };
  std::unordered_map<WsConnectionId, ConnectionState> connections_;

  bool tryParseFrame(std::vector<uint8_t>& buffer, WsFrame& outFrame) const;
};

} // namespace temene

#endif // WEBSOCKET_SERVER_H
