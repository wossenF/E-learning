#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>

namespace temene {

struct XmlAttribute {
  std::string name;
  std::string value;
};

class XmlElement {
public:
  std::string tagName;
  std::string textContent;
  std::vector<XmlAttribute> attributes;
  std::vector<std::shared_ptr<XmlElement>> children;

  std::string getAttribute(const std::string& name) const;
  void print(int indent = 0) const;
};

class XmlParser {
public:
  std::shared_ptr<XmlElement> parse(const std::string& xmlData);

private:
  size_t pos_{0};
  std::string data_;

  void skipWhitespace();
  std::shared_ptr<XmlElement> parseElement();
  std::vector<XmlAttribute> parseAttributes();
  std::string parseText();
};

} // namespace temene

#endif // XML_PARSER_H
