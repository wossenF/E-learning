#ifndef DNS_RESOLVER_H
#define DNS_RESOLVER_H

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <memory>

namespace temene {

enum class DnsType : uint16_t { A = 1, NS = 2, CNAME = 5, SOA = 6, PTR = 12, MX = 15, TXT = 16, AAAA = 28 };
enum class DnsClass : uint16_t { IN = 1 };

struct DnsHeader {
  uint16_t id;
  uint16_t flags;
  uint16_t qdcount;
  uint16_t ancount;
  uint16_t nscount;
  uint16_t arcount;
};

struct DnsQuestion {
  std::string qname;
  uint16_t qtype;
  uint16_t qclass;
};

struct DnsRecord {
  std::string name;
  uint16_t type;
  uint16_t clazz;
  uint32_t ttl;
  uint16_t rdlength;
  std::vector<uint8_t> rdata;
};

struct DnsMessage {
  DnsHeader header;
  std::vector<DnsQuestion> questions;
  std::vector<DnsRecord> answers;
  std::vector<DnsRecord> authorities;
  std::vector<DnsRecord> additionals;
};

class DnsResolver {
public:
  std::vector<uint8_t> serializeMessage(const DnsMessage& msg) const;
  bool parseMessage(const std::vector<uint8_t>& data, DnsMessage& outMsg) const;

  std::vector<std::string> resolveIPv4(const std::string& domainName);
  std::vector<std::string> resolveIPv6(const std::string& domainName);

private:
  std::unordered_map<std::string, std::vector<std::string>> mockCacheA_;
  std::unordered_map<std::string, std::vector<std::string>> mockCacheAAAA_;

  std::string parseName(const std::vector<uint8_t>& data, size_t& offset) const;
  void writeName(std::vector<uint8_t>& out, const std::string& name) const;
};

} // namespace temene

#endif // DNS_RESOLVER_H
