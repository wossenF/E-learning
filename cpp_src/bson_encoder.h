#ifndef BSON_ENCODER_H
#define BSON_ENCODER_H

#include <vector>
#include <string>
#include <unordered_map>
#include <variant>
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace temene {

enum class BsonType : uint8_t {
  BSON_DOUBLE = 0x01,
  BSON_STRING = 0x02,
  BSON_DOCUMENT = 0x03,
  BSON_ARRAY = 0x04,
  BSON_BINARY = 0x05,
  BSON_BOOLEAN = 0x08,
  BSON_DATETIME = 0x09,
  BSON_NULL = 0x0A,
  BSON_INT32 = 0x10,
  BSON_INT64 = 0x12
};

class BsonElement;

using BsonValue = std::variant<
  std::monostate,
  double,
  std::string,
  bool,
  int32_t,
  int64_t,
  std::vector<uint8_t>,
  std::shared_ptr<class BsonDocument>,
  std::shared_ptr<class BsonArray>
>;

class BsonDocument {
public:
  void putDouble(const std::string& key, double val);
  void putString(const std::string& key, const std::string& val);
  void putBool(const std::string& key, bool val);
  void putInt32(const std::string& key, int32_t val);
  void putInt64(const std::string& key, int64_t val);
  void putDocument(const std::string& key, std::shared_ptr<BsonDocument> doc);
  void putArray(const std::string& key, std::shared_ptr<BsonArray> arr);
  void putNull(const std::string& key);

  bool has(const std::string& key) const;
  BsonValue get(const std::string& key) const;
  const std::unordered_map<std::string, BsonValue>& getElements() const { return elements_; }

  std::vector<uint8_t> serialize() const;
  static std::shared_ptr<BsonDocument> deserialize(const std::vector<uint8_t>& buffer, size_t& offset);

private:
  std::unordered_map<std::string, BsonValue> elements_;
};

class BsonArray {
public:
  void appendDouble(double val);
  void appendString(const std::string& val);
  void appendBool(bool val);
  void appendInt32(int32_t val);
  void appendInt64(int64_t val);
  void appendDocument(std::shared_ptr<BsonDocument> doc);

  size_t size() const { return values_.size(); }
  BsonValue at(size_t index) const;

  std::vector<uint8_t> serialize() const;
  static std::shared_ptr<BsonArray> deserialize(const std::vector<uint8_t>& buffer, size_t& offset);

private:
  std::vector<BsonValue> values_;
};

class BsonEncoderUtils {
public:
  static void writeInt32(std::vector<uint8_t>& buf, int32_t val);
  static void writeInt64(std::vector<uint8_t>& buf, int64_t val);
  static void writeDouble(std::vector<uint8_t>& buf, double val);
  static void writeCString(std::vector<uint8_t>& buf, const std::string& str);

  static int32_t readInt32(const std::vector<uint8_t>& buf, size_t& offset);
  static int64_t readInt64(const std::vector<uint8_t>& buf, size_t& offset);
  static double readDouble(const std::vector<uint8_t>& buf, size_t& offset);
  static std::string readCString(const std::vector<uint8_t>& buf, size_t& offset);
};

} // namespace temene

#endif // BSON_ENCODER_H
