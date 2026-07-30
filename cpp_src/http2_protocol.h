#ifndef HTTP2_PROTOCOL_H
#define HTTP2_PROTOCOL_H

#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <memory>
#include <iostream>

namespace temene {

enum class Http2FrameType : uint8_t {
  DATA = 0x0,
  HEADERS = 0x1,
  PRIORITY = 0x2,
  RST_STREAM = 0x3,
  SETTINGS = 0x4,
  PUSH_PROMISE = 0x5,
  PING = 0x6,
  GOAWAY = 0x7,
  WINDOW_UPDATE = 0x8,
  CONTINUATION = 0x9
};

struct Http2FrameHeader {
  uint32_t length; // 24-bit
  Http2FrameType type;
  uint8_t flags;
  uint32_t streamId; // 31-bit
};

struct Http2Frame {
  Http2FrameHeader header;
  std::vector<uint8_t> payload;
};

class HpackEncoder {
public:
  std::vector<uint8_t> encode(const std::unordered_map<std::string, std::string>& headers);
  std::unordered_map<std::string, std::string> decode(const std::vector<uint8_t>& payload);

private:
  std::vector<std::pair<std::string, std::string>> dynamicTable_;
};

class Http2ProtocolEngine {
public:
  Http2ProtocolEngine();

  std::vector<uint8_t> serializeFrame(const Http2Frame& frame) const;
  bool parseFrame(const std::vector<uint8_t>& data, size_t& offset, Http2Frame& outFrame) const;

  std::vector<uint8_t> buildSettingsFrame() const;
  std::vector<uint8_t> buildHeadersFrame(uint32_t streamId, const std::unordered_map<std::string, std::string>& headers, bool endStream);
  std::vector<uint8_t> buildDataFrame(uint32_t streamId, const std::string& data, bool endStream);

private:
  HpackEncoder hpack_;
};

} // namespace temene

#endif // HTTP2_PROTOCOL_H
