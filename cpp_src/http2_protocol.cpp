#include "http2_protocol.h"
#include <cstring>
#include <stdexcept>

namespace temene {

std::vector<uint8_t> HpackEncoder::encode(const std::unordered_map<std::string, std::string>& headers) {
  std::vector<uint8_t> result;
  for (const auto& [k, v] : headers) {
    result.push_back(0x00); // Literal Header Field without Indexing
    result.push_back(static_cast<uint8_t>(k.size()));
    for (char c : k) result.push_back(c);
    result.push_back(static_cast<uint8_t>(v.size()));
    for (char c : v) result.push_back(c);
  }
  return result;
}

std::unordered_map<std::string, std::string> HpackEncoder::decode(const std::vector<uint8_t>& payload) {
  std::unordered_map<std::string, std::string> headers;
  size_t idx = 0;
  while (idx < payload.size()) {
    uint8_t type = payload[idx++];
    if (type == 0x00) {
      if (idx >= payload.size()) break;
      uint8_t kLen = payload[idx++];
      if (idx + kLen > payload.size()) break;
      std::string k(payload.begin() + idx, payload.begin() + idx + kLen);
      idx += kLen;

      if (idx >= payload.size()) break;
      uint8_t vLen = payload[idx++];
      if (idx + vLen > payload.size()) break;
      std::string v(payload.begin() + idx, payload.begin() + idx + vLen);
      idx += vLen;

      headers[k] = v;
    }
  }
  return headers;
}

Http2ProtocolEngine::Http2ProtocolEngine() {}

std::vector<uint8_t> Http2ProtocolEngine::serializeFrame(const Http2Frame& frame) const {
  std::vector<uint8_t> result(9 + frame.payload.size());

  uint32_t len = frame.header.length & 0x00FFFFFF;
  result[0] = static_cast<uint8_t>((len >> 16) & 0xFF);
  result[1] = static_cast<uint8_t>((len >> 8) & 0xFF);
  result[2] = static_cast<uint8_t>(len & 0xFF);

  result[3] = static_cast<uint8_t>(frame.header.type);
  result[4] = frame.header.flags;

  uint32_t streamId = frame.header.streamId & 0x7FFFFFFF;
  result[5] = static_cast<uint8_t>((streamId >> 24) & 0xFF);
  result[6] = static_cast<uint8_t>((streamId >> 16) & 0xFF);
  result[7] = static_cast<uint8_t>((streamId >> 8) & 0xFF);
  result[8] = static_cast<uint8_t>(streamId & 0xFF);

  std::memcpy(result.data() + 9, frame.payload.data(), frame.payload.size());
  return result;
}

bool Http2ProtocolEngine::parseFrame(const std::vector<uint8_t>& data, size_t& offset, Http2Frame& outFrame) const {
  if (data.size() - offset < 9) return false;

  uint32_t len = (data[offset] << 16) | (data[offset + 1] << 8) | data[offset + 2];
  if (data.size() - offset < 9 + len) return false;

  outFrame.header.length = len;
  outFrame.header.type = static_cast<Http2FrameType>(data[offset + 3]);
  outFrame.header.flags = data[offset + 4];
  outFrame.header.streamId = ((data[offset + 5] & 0x7F) << 24) | (data[offset + 6] << 16) | (data[offset + 7] << 8) | data[offset + 8];

  outFrame.payload.assign(data.begin() + offset + 9, data.begin() + offset + 9 + len);
  offset += 9 + len;
  return true;
}

std::vector<uint8_t> Http2ProtocolEngine::buildSettingsFrame() const {
  Http2Frame frame;
  frame.header.length = 0;
  frame.header.type = Http2FrameType::SETTINGS;
  frame.header.flags = 0;
  frame.header.streamId = 0;
  return serializeFrame(frame);
}

std::vector<uint8_t> Http2ProtocolEngine::buildHeadersFrame(
  uint32_t streamId,
  const std::unordered_map<std::string, std::string>& headers,
  bool endStream
) {
  Http2Frame frame;
  frame.payload = hpack_.encode(headers);
  frame.header.length = static_cast<uint32_t>(frame.payload.size());
  frame.header.type = Http2FrameType::HEADERS;
  frame.header.flags = 0x04; // END_HEADERS
  if (endStream) frame.header.flags |= 0x01; // END_STREAM
  frame.header.streamId = streamId;
  return serializeFrame(frame);
}

std::vector<uint8_t> Http2ProtocolEngine::buildDataFrame(
  uint32_t streamId,
  const std::string& data,
  bool endStream
) {
  Http2Frame frame;
  frame.payload.assign(data.begin(), data.end());
  frame.header.length = static_cast<uint32_t>(frame.payload.size());
  frame.header.type = Http2FrameType::DATA;
  frame.header.flags = 0;
  if (endStream) frame.header.flags |= 0x01; // END_STREAM
  frame.header.streamId = streamId;
  return serializeFrame(frame);
}

} // namespace temene
