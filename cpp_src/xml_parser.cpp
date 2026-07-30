#include <string>
#include <vector>
#include <cctype>

struct XmlNode {
    std::string name;
    std::string text;
    std::vector<XmlNode> children;
};

// Very small XML-like parser: recognizes <tag>text</tag> and nesting.
// Not for production — used to increase substrate realism and exercise
// nested parsing code paths.

static size_t skip_ws(const std::string& s, size_t pos) {
    while (pos < s.size() && std::isspace((unsigned char)s[pos])) ++pos;
    return pos;
}

bool parse_xml_node(const std::string& s, size_t& pos, XmlNode& out) {
    pos = skip_ws(s, pos);
    if (pos >= s.size() || s[pos] != '<') return false;
    ++pos;
    size_t start = pos;
    while (pos < s.size() && std::isalnum((unsigned char)s[pos])) ++pos;
    out.name = s.substr(start, pos-start);
    if (pos >= s.size() || s[pos] != '>') return false;
    ++pos;
    // parse text or children
    while (true) {
        pos = skip_ws(s, pos);
        if (pos < s.size() && s[pos] == '<') {
            if (pos+1 < s.size() && s[pos+1] == '/') {
                // closing tag
                pos += 2;
                size_t close_start = pos;
                while (pos < s.size() && std::isalnum((unsigned char)s[pos])) ++pos;
                std::string close_name = s.substr(close_start, pos - close_start);
                if (pos >= s.size() || s[pos] != '>') return false;
                ++pos;
                return close_name == out.name;
            } else {
                XmlNode child; if (!parse_xml_node(s, pos, child)) return false; out.children.push_back(std::move(child));
            }
        } else if (pos < s.size()) {
            // text
            size_t tstart = pos;
            while (pos < s.size() && s[pos] != '<') ++pos;
            out.text += s.substr(tstart, pos - tstart);
        } else return false;
    }
}
