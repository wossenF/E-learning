#include "tokenizer.h"
#include <cctype>
#include <cstdlib>

Tokenizer::Tokenizer(const char* data, size_t len)
    : p_(data), end_(data + len), have_lookahead_(false) {}

void Tokenizer::skip_ws() {
    while (p_ < end_ && std::isspace((unsigned char)*p_)) ++p_;
}

Token Tokenizer::read_string() {
    Token t{TokenType::Error, std::string(), static_cast<size_t>(p_ - (end_ - (end_ - p_)))};
    if (p_==end_ || *p_ != '"') { t.type = TokenType::Error; return t; }
    ++p_;
    std::string out;
    while (p_ < end_) {
        char c = *p_++;
        if (c == '\\') {
            if (p_==end_) break;
            char esc = *p_++;
            switch (esc) {
                case 'n': out.push_back('\n'); break;
                case 't': out.push_back('\t'); break;
                case 'r': out.push_back('\r'); break;
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                default: out.push_back(esc); break;
            }
        } else if (c == '"') {
            t.type = TokenType::String;
            t.text = std::move(out);
            return t;
        } else {
            out.push_back(c);
        }
    }
    t.type = TokenType::Error;
    return t;
}

Token Tokenizer::read_number() {
    const char* start = p_;
    if (p_ < end_ && (*p_=='-' || *p_=='+')) ++p_;
    while (p_ < end_ && std::isdigit((unsigned char)*p_)) ++p_;
    if (p_ < end_ && *p_ == '.') {
        ++p_;
        while (p_ < end_ && std::isdigit((unsigned char)*p_)) ++p_;
    }
    size_t len = static_cast<size_t>(p_ - start);
    Token t{TokenType::Number, std::string(start, len), static_cast<size_t>(start - (end_ - (end_ - p_)))};
    return t;
}

Token Tokenizer::next() {
    if (have_lookahead_) { have_lookahead_ = false; return lookahead_; }
    skip_ws();
    if (p_ >= end_) return Token{TokenType::End, std::string(), static_cast<size_t>(p_ - (end_ - (end_ - p_)))};
    char c = *p_++;
    switch (c) {
        case '{': return Token{TokenType::LeftBrace, "", static_cast<size_t>(p_ - (end_ - (end_ - p_)))};
        case '}': return Token{TokenType::RightBrace, "", static_cast<size_t>(p_ - (end_ - (end_ - p_)))};
        case '[': return Token{TokenType::LeftBracket, "", static_cast<size_t>(p_ - (end_ - (end_ - p_)))};
        case ']': return Token{TokenType::RightBracket, "", static_cast<size_t>(p_ - (end_ - (end_ - p_)))};
        case ':': return Token{TokenType::Colon, "", static_cast<size_t>(p_ - (end_ - (end_ - p_)))};
        case ',': return Token{TokenType::Comma, "", static_cast<size_t>(p_ - (end_ - (end_ - p_)))};
        case '"': --p_; return read_string();
        default:
            if (std::isdigit((unsigned char)c) || c == '-' || c == '+') { --p_; return read_number(); }
            else if (c=='t') { // true
                if (p_+2 < end_ && p_[0]=='r' && p_[1]=='u') { p_ += 2; return Token{TokenType::True, "true", 0}; }
            }
            else if (c=='f') { // false
                if (p_+3 < end_ && p_[0]=='a' && p_[1]=='l' && p_[2]=='s') { p_ += 3; return Token{TokenType::False, "false", 0}; }
            }
            else if (c=='n') { // null
                if (p_+2 < end_ && p_[0]=='u' && p_[1]=='l') { p_ += 2; return Token{TokenType::Null, "null", 0}; }
            }
            break;
    }
    return Token{TokenType::Error, std::string(1,c), 0};
}

Token Tokenizer::peek() const {
    if (have_lookahead_) return lookahead_;
    return Token{TokenType::Error, std::string(), 0};
}

bool Tokenizer::eof() const { return p_ >= end_; }
