#include "dns_resolver.h"
#include <cstring>
#include <sstream>

namespace temene {

std::string DnsResolver::parseName(const std::vector<uint8_t>& data, size_t& offset) const {
  std::string name;
  while (offset < data.size()) {
    uint8_t len = data[offset++];
    if (len == 0) break;
    
    if ((len & 0xC0) == 0xC0) {
      if (offset >= data.size()) break;
      uint16_t ptrOffset = ((len & 0x3F) << 8) | data[offset++];
      size_t saveOffset = offset;
      offset = ptrOffset;
      name += parseName(data, offset);
      offset = saveOffset;
      return name;
    } else {
      if (offset + len > data.size()) break;
      if (!name.empty()) name += ".";
      name.append(data.begin() + offset, data.begin() + offset + len);
      offset += len;
    }
  }
  return name;
}

void DnsResolver::writeName(std::vector<uint8_t>& out, const std::string& name) const {
  size_t start = 0;
  while (start < name.size()) {
    size_t end = name.find('.', start);
    if (end == std::string::npos) end = name.size();
    
    size_t len = end - start;
    out.push_back(static_cast<uint8_t>(len));
    for (size_t i = start; i < end; ++i) {
      out.push_back(static_cast<uint8_t>(name[i]));
    }
    
    start = end + 1;
  }
  out.push_back(0);
}

std::vector<uint8_t> DnsResolver::serializeMessage(const DnsMessage& msg) const {
  std::vector<uint8_t> out;
  
  out.push_back(static_cast<uint8_t>(msg.header.id >> 8));
  out.push_back(static_cast<uint8_t>(msg.header.id & 0xFF));
  out.push_back(static_cast<uint8_t>(msg.header.flags >> 8));
  out.push_back(static_cast<uint8_t>(msg.header.flags & 0xFF));
  out.push_back(static_cast<uint8_t>(msg.header.qdcount >> 8));
  out.push_back(static_cast<uint8_t>(msg.header.qdcount & 0xFF));
  out.push_back(static_cast<uint8_t>(msg.header.ancount >> 8));
  out.push_back(static_cast<uint8_t>(msg.header.ancount & 0xFF));
  out.push_back(static_cast<uint8_t>(msg.header.nscount >> 8));
  out.push_back(static_cast<uint8_t>(msg.header.nscount & 0xFF));
  out.push_back(static_cast<uint8_t>(msg.header.arcount >> 8));
  out.push_back(static_cast<uint8_t>(msg.header.arcount & 0xFF));
  
  for (const auto& q : msg.questions) {
    writeName(out, q.qname);
    out.push_back(static_cast<uint8_t>(q.qtype >> 8));
    out.push_back(static_cast<uint8_t>(q.qtype & 0xFF));
    out.push_back(static_cast<uint8_t>(q.qclass >> 8));
    out.push_back(static_cast<uint8_t>(q.qclass & 0xFF));
  }
  
  auto writeRecord = [&](const DnsRecord& r) {
    writeName(out, r.name);
    out.push_back(static_cast<uint8_t>(r.type >> 8));
    out.push_back(static_cast<uint8_t>(r.type & 0xFF));
    out.push_back(static_cast<uint8_t>(r.clazz >> 8));
    out.push_back(static_cast<uint8_t>(r.clazz & 0xFF));
    out.push_back(static_cast<uint8_t>((r.ttl >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((r.ttl >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((r.ttl >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(r.ttl & 0xFF));
    out.push_back(static_cast<uint8_t>(r.rdlength >> 8));
    out.push_back(static_cast<uint8_t>(r.rdlength & 0xFF));
    out.insert(out.end(), r.rdata.begin(), r.rdata.end());
  };
  
  for (const auto& r : msg.answers) writeRecord(r);
  for (const auto& r : msg.authorities) writeRecord(r);
  for (const auto& r : msg.additionals) writeRecord(r);
  
  return out;
}

bool DnsResolver::parseMessage(const std::vector<uint8_t>& data, DnsMessage& outMsg) const {
  if (data.size() < 12) return false;
  
  size_t offset = 0;
  outMsg.header.id = (data[offset] << 8) | data[offset + 1]; offset += 2;
  outMsg.header.flags = (data[offset] << 8) | data[offset + 1]; offset += 2;
  outMsg.header.qdcount = (data[offset] << 8) | data[offset + 1]; offset += 2;
  outMsg.header.ancount = (data[offset] << 8) | data[offset + 1]; offset += 2;
  outMsg.header.nscount = (data[offset] << 8) | data[offset + 1]; offset += 2;
  outMsg.header.arcount = (data[offset] << 8) | data[offset + 1]; offset += 2;
  
  for (int i = 0; i < outMsg.header.qdcount; ++i) {
    DnsQuestion q;
    q.qname = parseName(data, offset);
    if (offset + 4 > data.size()) return false;
    q.qtype = (data[offset] << 8) | data[offset + 1]; offset += 2;
    q.qclass = (data[offset] << 8) | data[offset + 1]; offset += 2;
    outMsg.questions.push_back(q);
  }
  
  auto parseRecord = [&](DnsRecord& r) -> bool {
    r.name = parseName(data, offset);
    if (offset + 10 > data.size()) return false;
    r.type = (data[offset] << 8) | data[offset + 1]; offset += 2;
    r.clazz = (data[offset] << 8) | data[offset + 1]; offset += 2;
    r.ttl = (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3]; offset += 4;
    r.rdlength = (data[offset] << 8) | data[offset + 1]; offset += 2;
    
    if (offset + r.rdlength > data.size()) return false;
    r.rdata.assign(data.begin() + offset, data.begin() + offset + r.rdlength);
    offset += r.rdlength;
    return true;
  };
  
  for (int i = 0; i < outMsg.header.ancount; ++i) {
    DnsRecord r; if (!parseRecord(r)) return false; outMsg.answers.push_back(r);
  }
  for (int i = 0; i < outMsg.header.nscount; ++i) {
    DnsRecord r; if (!parseRecord(r)) return false; outMsg.authorities.push_back(r);
  }
  for (int i = 0; i < outMsg.header.arcount; ++i) {
    DnsRecord r; if (!parseRecord(r)) return false; outMsg.additionals.push_back(r);
  }
  
  return true;
}

std::vector<std::string> DnsResolver::resolveIPv4(const std::string& domainName) {
  auto it = mockCacheA_.find(domainName);
  if (it != mockCacheA_.end()) return it->second;
  return {"93.184.216.34"}; // default example.com
}

std::vector<std::string> DnsResolver::resolveIPv6(const std::string& domainName) {
  auto it = mockCacheAAAA_.find(domainName);
  if (it != mockCacheAAAA_.end()) return it->second;
  return {"2606:2800:220:1:248:1893:25c8:1946"}; // default example.com
}

} // namespace temene
