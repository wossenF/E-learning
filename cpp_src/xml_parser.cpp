#include "xml_parser.h"
#include <cctype>

namespace temene {

std::string XmlElement::getAttribute(const std::string& name) const {
  for (const auto& attr : attributes) {
    if (attr.name == name) return attr.value;
  }
  return "";
}

void XmlElement::print(int indent) const {
  std::string ind(indent, ' ');
  std::cout << ind << "<" << tagName;
  for (const auto& attr : attributes) {
    std::cout << " " << attr.name << "=\"" << attr.value << "\"";
  }
  if (children.empty() && textContent.empty()) {
    std::cout << "/>\n";
  } else {
    std::cout << ">\n";
    if (!textContent.empty()) {
      std::cout << ind << "  " << textContent << "\n";
    }
    for (const auto& child : children) {
      child->print(indent + 2);
    }
    std::cout << ind << "</" << tagName << ">\n";
  }
}

void XmlParser::skipWhitespace() {
  while (pos_ < data_.size() && std::isspace(data_[pos_])) {
    pos_++;
  }
}

std::shared_ptr<XmlElement> XmlParser::parse(const std::string& xmlData) {
  data_ = xmlData;
  pos_ = 0;
  skipWhitespace();
  
  if (pos_ < data_.size() && data_.compare(pos_, 5, "<?xml") == 0) {
    size_t endDecl = data_.find("?>", pos_);
    if (endDecl != std::string::npos) pos_ = endDecl + 2;
  }
  
  skipWhitespace();
  return parseElement();
}

std::vector<XmlAttribute> XmlParser::parseAttributes() {
  std::vector<XmlAttribute> attributes;
  while (pos_ < data_.size() && data_[pos_] != '>' && data_[pos_] != '/') {
    skipWhitespace();
    if (data_[pos_] == '>' || data_[pos_] == '/') break;
    
    size_t eqPos = data_.find('=', pos_);
    if (eqPos == std::string::npos) break;
    
    std::string name = data_.substr(pos_, eqPos - pos_);
    pos_ = eqPos + 1;
    skipWhitespace();
    
    if (data_[pos_] == '"') {
      pos_++;
      size_t endQuote = data_.find('"', pos_);
      if (endQuote != std::string::npos) {
        std::string val = data_.substr(pos_, endQuote - pos_);
        attributes.push_back({name, val});
        pos_ = endQuote + 1;
      }
    }
  }
  return attributes;
}

std::string XmlParser::parseText() {
  size_t start = pos_;
  while (pos_ < data_.size() && data_[pos_] != '<') {
    pos_++;
  }
  std::string txt = data_.substr(start, pos_ - start);
  
  // Trim
  size_t first = txt.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return "";
  size_t last = txt.find_last_not_of(" \t\r\n");
  return txt.substr(first, (last - first + 1));
}

std::shared_ptr<XmlElement> XmlParser::parseElement() {
  if (pos_ >= data_.size() || data_[pos_] != '<') return nullptr;
  pos_++; // skip '<'
  
  size_t nameEnd = pos_;
  while (nameEnd < data_.size() && !std::isspace(data_[nameEnd]) && data_[nameEnd] != '>' && data_[nameEnd] != '/') {
    nameEnd++;
  }
  
  auto element = std::make_shared<XmlElement>();
  element->tagName = data_.substr(pos_, nameEnd - pos_);
  pos_ = nameEnd;
  
  element->attributes = parseAttributes();
  skipWhitespace();
  
  if (data_[pos_] == '/') {
    pos_ += 2; // skip "/>"
    return element;
  }
  
  pos_++; // skip '>'
  
  while (pos_ < data_.size()) {
    skipWhitespace();
    if (data_[pos_] == '<') {
      if (pos_ + 1 < data_.size() && data_[pos_ + 1] == '/') {
        // End tag
        pos_ += 2;
        size_t endTagClose = data_.find('>', pos_);
        if (endTagClose != std::string::npos) {
          pos_ = endTagClose + 1;
        }
        break;
      } else {
        auto child = parseElement();
        if (child) element->children.push_back(child);
      }
    } else {
      std::string text = parseText();
      if (!text.empty()) {
        element->textContent = text;
      }
    }
  }
  
  return element;
}

} // namespace temene
