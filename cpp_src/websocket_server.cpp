#include "websocket_server.h"
#include <cstring>
#include <stdexcept>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

namespace temene {

WebSocketServer::WebSocketServer() {}

void WebSocketServer::setMessageHandler(WsMessageHandler handler) {
  onMessage_ = handler;
}

void WebSocketServer::setCloseHandler(WsCloseHandler handler) {
  onClose_ = handler;
}

std::vector<uint8_t> WebSocketServer::generateAcceptKey(const std::string& clientKey) const {
  std::string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  std::string combined = clientKey + magic;

  unsigned char hash[SHA_1_LENGTH];
  SHA1(reinterpret_cast<const unsigned char*>(combined.data()), combined.size(), hash);

  BIO *b64 = BIO_new(BIO_f_base64());
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  BIO *bmem = BIO_new(BIO_s_mem());
  b64 = BIO_push(b64, bmem);

  BIO_write(b64, hash, SHA_1_LENGTH);
  BIO_flush(b64);

  BUF_MEM *bptr;
  BIO_get_mem_ptr(b64, &bptr);

  std::vector<uint8_t> result(bptr->data, bptr->data + bptr->length);
  BIO_free_all(b64);

  return result;
}

bool WebSocketServer::tryParseFrame(std::vector<uint8_t>& buffer, WsFrame& outFrame) const {
  if (buffer.size() < 2) return false;

  outFrame.fin = (buffer[0] & 0x80) != 0;
  outFrame.opcode = static_cast<WsOpcode>(buffer[0] & 0x0F);
  outFrame.masked = (buffer[1] & 0x80) != 0;

  uint64_t payloadLen = buffer[1] & 0x7F;
  size_t offset = 2;

  if (payloadLen == 126) {
    if (buffer.size() < 4) return false;
    payloadLen = (buffer[2] << 8) | buffer[3];
    offset += 2;
  } else if (payloadLen == 127) {
    if (buffer.size() < 10) return false;
    payloadLen = 0;
    for (int i = 0; i < 8; ++i) {
      payloadLen = (payloadLen << 8) | buffer[2 + i];
    }
    offset += 8;
  }

  if (outFrame.masked) {
    if (buffer.size() < offset + 4) return false;
    outFrame.maskingKey.assign(buffer.begin() + offset, buffer.begin() + offset + 4);
    offset += 4;
  }

  if (buffer.size() < offset + payloadLen) return false;

  outFrame.payload.assign(buffer.begin() + offset, buffer.begin() + offset + payloadLen);

  if (outFrame.masked) {
    for (size_t i = 0; i < outFrame.payload.size(); ++i) {
      outFrame.payload[i] ^= outFrame.maskingKey[i % 4];
    }
  }

  buffer.erase(buffer.begin(), buffer.begin() + offset + payloadLen);
  return true;
}

void WebSocketServer::handleIncomingData(WsConnectionId connId, const std::vector<uint8_t>& data) {
  auto& state = connections_[connId];
  state.buffer.insert(state.buffer.end(), data.begin(), data.end());

  WsFrame frame;
  while (tryParseFrame(state.buffer, frame)) {
    if (frame.opcode == WsOpcode::CLOSE) {
      if (onClose_) onClose_(connId);
      connections_.erase(connId);
      break;
    } else if (frame.opcode == WsOpcode::PING) {
      // Automatic pong could be sent here
    } else if (frame.opcode == WsOpcode::TEXT || frame.opcode == WsOpcode::BINARY) {
      if (onMessage_) onMessage_(connId, frame.payload, frame.opcode == WsOpcode::TEXT);
    }
  }
}

std::vector<uint8_t> WebSocketServer::buildFrame(const std::vector<uint8_t>& payload, WsOpcode opcode, bool fin) const {
  std::vector<uint8_t> frame;
  frame.push_back((fin ? 0x80 : 0x00) | static_cast<uint8_t>(opcode));

  if (payload.size() < 126) {
    frame.push_back(static_cast<uint8_t>(payload.size()));
  } else if (payload.size() <= 0xFFFF) {
    frame.push_back(126);
    frame.push_back(static_cast<uint8_t>((payload.size() >> 8) & 0xFF));
    frame.push_back(static_cast<uint8_t>(payload.size() & 0xFF));
  } else {
    frame.push_back(127);
    for (int i = 7; i >= 0; --i) {
      frame.push_back(static_cast<uint8_t>((payload.size() >> (i * 8)) & 0xFF));
    }
  }

  frame.insert(frame.end(), payload.begin(), payload.end());
  return frame;
}

} // namespace temene
