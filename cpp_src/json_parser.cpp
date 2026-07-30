#include "json_parser.h"
#include <string>
#include <cctype>
#include <cstdlib>
#include <sstream>

static const char* skip_ws(const char* p, const char* end) {
    while (p < end && std::isspace((unsigned char)*p)) ++p;
    return p;
}

bool parse_string(const char*& p, const char* end, std::string& out) {
    if (p==end || *p != '"') return false;
    ++p; // skip '"'
    while (p < end) {
        char c = *p++;
        if (c == '\\') {
            if (p==end) return false;
            char esc = *p++;
            switch (esc) {
                case 'n': out.push_back('\n'); break;
                case 't': out.push_back('\t'); break;
                case 'r': out.push_back('\r'); break;
                case '\\': out.push_back('\\'); break;
                case '"': out.push_back('"'); break;
                default: out.push_back(esc); break;
            }
        } else if (c == '"') {
            return true;
        } else {
            out.push_back(c);
        }
    }
    return false;
}

bool parse_number(const char*& p, const char* end, double& out) {
    const char* start = p;
    while (p < end && (std::isdigit((unsigned char)*p) || *p=='-' || *p=='+' || *p=='.' || *p=='e' || *p=='E')) ++p;
    if (p==start) return false;
    try {
        out = std::strtod(start, nullptr);
    } catch (...) { return false; }
    return true;
}

bool parse_array(const char*& p, const char* end) {
    if (p==end || *p!='[') return false;
    ++p;
    p = skip_ws(p, end);
    while (p < end && *p != ']') {
        // try to parse a string or number or nested array (shallow)
        if (*p == '"') { std::string s; if (!parse_string(p, end, s)) return false; }
        else if (*p == '[') { if (!parse_array(p, end)) return false; }
        else { double num; if (!parse_number(p, end, num)) return false; }
        p = skip_ws(p, end);
        if (*p == ',') { ++p; p = skip_ws(p, end); continue; }
        break;
    }
    if (p==end || *p != ']') return false;
    ++p;
    return true;
}

bool parse_object(const char*& p, const char* end) {
    if (p==end || *p!='{') return false;
    ++p;
    p = skip_ws(p, end);
    while (p < end && *p != '}') {
        std::string key;
        if (!parse_string(p, end, key)) return false;
        p = skip_ws(p, end);
        if (p==end || *p!=':') return false;
        ++p; p = skip_ws(p, end);
        if (*p == '"') { std::string v; if (!parse_string(p, end, v)) return false; }
        else if (*p == '{') { if (!parse_object(p, end)) return false; }
        else if (*p == '[') { if (!parse_array(p, end)) return false; }
        else { double num; if (!parse_number(p, end, num)) return false; }
        p = skip_ws(p, end);
        if (*p == ',') { ++p; p = skip_ws(p, end); continue; }
        break;
    }
    if (p==end || *p != '}') return false;
    ++p;
    return true;
}

bool parse_json_like(const uint8_t* data, size_t size) {
    if (!data || size == 0) return false;
    const char* p = reinterpret_cast<const char*>(data);
    const char* end = p + size;
    p = skip_ws(p, end);
    if (p==end) return false;
    bool ok = false;
    if (*p == '{') ok = parse_object(p, end);
    else if (*p == '[') ok = parse_array(p, end);
    else if (*p == '"') { std::string s; ok = parse_string(p, end, s); }
    else { double n; ok = parse_number(p, end, n); }
    return ok;
}
