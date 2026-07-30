#pragma once
#include <string>
#include <vector>
#include <cstddef>

enum class TokenType {
    LeftBrace, RightBrace, LeftBracket, RightBracket,
    Colon, Comma,
    String, Number, True, False, Null,
    End, Error
};

struct Token {
    TokenType type;
    std::string text; // for String/Number
    size_t pos;
};

class Tokenizer {
public:
    Tokenizer(const char* data, size_t len);
    Token next();
    Token peek() const;
    bool eof() const;
private:
    const char* p_;
    const char* end_;
    Token lookahead_;
    bool have_lookahead_;

    void skip_ws();
    Token read_string();
    Token read_number();
};
