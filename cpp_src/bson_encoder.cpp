#include "bson_encoder.h"
#include <cstring>
#include <iostream>

namespace temene {

void BsonEncoderUtils::writeInt32(std::vector<uint8_t>& buf, int32_t val) {
  uint8_t bytes[4];
  std::memcpy(bytes, &val, 4);
  buf.insert(buf.end(), bytes, bytes + 4);
}

void BsonEncoderUtils::writeInt64(std::vector<uint8_t>& buf, int64_t val) {
  uint8_t bytes[8];
  std::memcpy(bytes, &val, 8);
  buf.insert(buf.end(), bytes, bytes + 8);
}

void BsonEncoderUtils::writeDouble(std::vector<uint8_t>& buf, double val) {
  uint8_t bytes[8];
  std::memcpy(bytes, &val, 8);
  buf.insert(buf.end(), bytes, bytes + 8);
}

void BsonEncoderUtils::writeCString(std::vector<uint8_t>& buf, const std::string& str) {
  buf.insert(buf.end(), str.begin(), str.end());
  buf.push_back(0x00);
}

int32_t BsonEncoderUtils::readInt32(const std::vector<uint8_t>& buf, size_t& offset) {
  if (offset + 4 > buf.size()) throw std::runtime_error("BSON read overflow for Int32.");
  int32_t val;
  std::memcpy(&val, &buf[offset], 4);
  offset += 4;
  return val;
}

int64_t BsonEncoderUtils::readInt64(const std::vector<uint8_t>& buf, size_t& offset) {
  if (offset + 8 > buf.size()) throw std::runtime_error("BSON read overflow for Int64.");
  int64_t val;
  std::memcpy(&val, &buf[offset], 8);
  offset += 8;
  return val;
}

double BsonEncoderUtils::readDouble(const std::vector<uint8_t>& buf, size_t& offset) {
  if (offset + 8 > buf.size()) throw std::runtime_error("BSON read overflow for Double.");
  double val;
  std::memcpy(&val, &buf[offset], 8);
  offset += 8;
  return val;
}

std::string BsonEncoderUtils::readCString(const std::vector<uint8_t>& buf, size_t& offset) {
  size_t start = offset;
  while (offset < buf.size() && buf[offset] != 0x00) {
    offset++;
  }
  if (offset >= buf.size()) throw std::runtime_error("Unterminated CString in BSON.");
  std::string str(buf.begin() + start, buf.begin() + offset);
  offset++; // Consume null byte
  return str;
}

// BsonDocument implementation
void BsonDocument::putDouble(const std::string& key, double val) { elements_[key] = val; }
void BsonDocument::putString(const std::string& key, const std::string& val) { elements_[key] = val; }
void BsonDocument::putBool(const std::string& key, bool val) { elements_[key] = val; }
void BsonDocument::putInt32(const std::string& key, int32_t val) { elements_[key] = val; }
void BsonDocument::putInt64(const std::string& key, int64_t val) { elements_[key] = val; }
void BsonDocument::putDocument(const std::string& key, std::shared_ptr<BsonDocument> doc) { elements_[key] = doc; }
void BsonDocument::putArray(const std::string& key, std::shared_ptr<BsonArray> arr) { elements_[key] = arr; }
void BsonDocument::putNull(const std::string& key) { elements_[key] = std::monostate{}; }

bool BsonDocument::has(const std::string& key) const {
  return elements_.find(key) != elements_.end();
}

BsonValue BsonDocument::get(const std::string& key) const {
  auto it = elements_.find(key);
  if (it != elements_.end()) return it->second;
  return std::monostate{};
}

std::vector<uint8_t> BsonDocument::serialize() const {
  std::vector<uint8_t> body;

  for (const auto& [key, val] : elements_) {
    if (std::holds_alternative<double>(val)) {
      body.push_back(static_cast<uint8_t>(BsonType::BSON_DOUBLE));
      BsonEncoderUtils::writeCString(body, key);
      BsonEncoderUtils::writeDouble(body, std::get<double>(val));
    } else if (std::holds_alternative<std::string>(val)) {
      body.push_back(static_cast<uint8_t>(BsonType::BSON_STRING));
      BsonEncoderUtils::writeCString(body, key);
      std::string str = std::get<std::string>(val);
      BsonEncoderUtils::writeInt32(body, static_cast<int32_t>(str.size() + 1));
      BsonEncoderUtils::writeCString(body, str);
    } else if (std::holds_alternative<bool>(val)) {
      body.push_back(static_cast<uint8_t>(BsonType::BSON_BOOLEAN));
      BsonEncoderUtils::writeCString(body, key);
      body.push_back(std::get<bool>(val) ? 0x01 : 0x00);
    } else if (std::holds_alternative<int32_t>(val)) {
      body.push_back(static_cast<uint8_t>(BsonType::BSON_INT32));
      BsonEncoderUtils::writeCString(body, key);
      BsonEncoderUtils::writeInt32(body, std::get<int32_t>(val));
    } else if (std::holds_alternative<int64_t>(val)) {
      body.push_back(static_cast<uint8_t>(BsonType::BSON_INT64));
      BsonEncoderUtils::writeCString(body, key);
      BsonEncoderUtils::writeInt64(body, std::get<int64_t>(val));
    } else if (std::holds_alternative<std::shared_ptr<BsonDocument>>(val)) {
      body.push_back(static_cast<uint8_t>(BsonType::BSON_DOCUMENT));
      BsonEncoderUtils::writeCString(body, key);
      auto subDoc = std::get<std::shared_ptr<BsonDocument>>(val);
      auto subBytes = subDoc->serialize();
      body.insert(body.end(), subBytes.begin(), subBytes.end());
    } else if (std::holds_alternative<std::shared_ptr<BsonArray>>(val)) {
      body.push_back(static_cast<uint8_t>(BsonType::BSON_ARRAY));
      BsonEncoderUtils::writeCString(body, key);
      auto subArr = std::get<std::shared_ptr<BsonArray>>(val);
      auto subBytes = subArr->serialize();
      body.insert(body.end(), subBytes.begin(), subBytes.end());
    } else if (std::holds_alternative<std::monostate>(val)) {
      body.push_back(static_cast<uint8_t>(BsonType::BSON_NULL));
      BsonEncoderUtils::writeCString(body, key);
    }
  }

  body.push_back(0x00); // Terminal null byte

  int32_t totalSize = static_cast<int32_t>(4 + body.size());
  std::vector<uint8_t> result;
  result.reserve(totalSize);
  BsonEncoderUtils::writeInt32(result, totalSize);
  result.insert(result.end(), body.begin(), body.end());

  return result;
}

std::shared_ptr<BsonDocument> BsonDocument::deserialize(const std::vector<uint8_t>& buffer, size_t& offset) {
  size_t startOffset = offset;
  int32_t totalSize = BsonEncoderUtils::readInt32(buffer, offset);

  auto doc = std::make_shared<BsonDocument>();

  while (offset < startOffset + totalSize - 1) {
    uint8_t typeByte = buffer[offset++];
    if (typeByte == 0x00) break;

    std::string key = BsonEncoderUtils::readCString(buffer, offset);
    BsonType btype = static_cast<BsonType>(typeByte);

    switch (btype) {
      case BsonType::BSON_DOUBLE:
        doc->putDouble(key, BsonEncoderUtils::readDouble(buffer, offset));
        break;
      case BsonType::BSON_STRING: {
        int32_t strLen = BsonEncoderUtils::readInt32(buffer, offset);
        std::string s = BsonEncoderUtils::readCString(buffer, offset);
        doc->putString(key, s);
        break;
      }
      case BsonType::BSON_BOOLEAN:
        doc->putBool(key, buffer[offset++] != 0x00);
        break;
      case BsonType::BSON_INT32:
        doc->putInt32(key, BsonEncoderUtils::readInt32(buffer, offset));
        break;
      case BsonType::BSON_INT64:
        doc->putInt64(key, BsonEncoderUtils::readInt64(buffer, offset));
        break;
      case BsonType::BSON_DOCUMENT: {
        auto subDoc = BsonDocument::deserialize(buffer, offset);
        doc->putDocument(key, subDoc);
        break;
      }
      case BsonType::BSON_ARRAY: {
        auto subArr = BsonArray::deserialize(buffer, offset);
        doc->putArray(key, subArr);
        break;
      }
      case BsonType::BSON_NULL:
        doc->putNull(key);
        break;
      default:
        throw std::runtime_error("Unsupported BSON type byte.");
    }
  }

  offset = startOffset + totalSize;
  return doc;
}

// BsonArray implementation
void BsonArray::appendDouble(double val) { values_.push_back(val); }
void BsonArray::appendString(const std::string& val) { values_.push_back(val); }
void BsonArray::appendBool(bool val) { values_.push_back(val); }
void BsonArray::appendInt32(int32_t val) { values_.push_back(val); }
void BsonArray::appendInt64(int64_t val) { values_.push_back(val); }
void BsonArray::appendDocument(std::shared_ptr<BsonDocument> doc) { values_.push_back(doc); }

BsonValue BsonArray::at(size_t index) const {
  if (index >= values_.size()) return std::monostate{};
  return values_[index];
}

std::vector<uint8_t> BsonArray::serialize() const {
  BsonDocument doc;
  for (size_t i = 0; i < values_.size(); ++i) {
    std::string key = std::to_string(i);
    const auto& val = values_[i];
    if (std::holds_alternative<double>(val)) doc.putDouble(key, std::get<double>(val));
    else if (std::holds_alternative<std::string>(val)) doc.putString(key, std::get<std::string>(val));
    else if (std::holds_alternative<bool>(val)) doc.putBool(key, std::get<bool>(val));
    else if (std::holds_alternative<int32_t>(val)) doc.putInt32(key, std::get<int32_t>(val));
    else if (std::holds_alternative<int64_t>(val)) doc.putInt64(key, std::get<int64_t>(val));
    else if (std::holds_alternative<std::shared_ptr<BsonDocument>>(val)) doc.putDocument(key, std::get<std::shared_ptr<BsonDocument>>(val));
  }
  return doc.serialize();
}

std::shared_ptr<BsonArray> BsonArray::deserialize(const std::vector<uint8_t>& buffer, size_t& offset) {
  auto doc = BsonDocument::deserialize(buffer, offset);
  auto arr = std::make_shared<BsonArray>();
  for (size_t i = 0; ; ++i) {
    std::string key = std::to_string(i);
    if (!doc->has(key)) break;
    arr->values_.push_back(doc->get(key));
  }
  return arr;
}

} // namespace temene
