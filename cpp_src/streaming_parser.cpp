#include "streaming_parser.h"
#include <cctype>
#include <cstdlib>

StreamingParser::StreamingParser() : state_(State::Root) {}

void StreamingParser::set_on_start_object(OnStartObject cb) { on_start_object_ = std::move(cb); }
void StreamingParser::set_on_end_object(OnEndObject cb) { on_end_object_ = std::move(cb); }
void StreamingParser::set_on_start_array(OnStartArray cb) { on_start_array_ = std::move(cb); }
void StreamingParser::set_on_end_array(OnEndArray cb) { on_end_array_ = std::move(cb); }
void StreamingParser::set_on_key(OnKey cb) { on_key_ = std::move(cb); }
void StreamingParser::set_on_string(OnString cb) { on_string_ = std::move(cb); }
void StreamingParser::set_on_number(OnNumber cb) { on_number_ = std::move(cb); }
void StreamingParser::set_on_bool(OnBool cb) { on_bool_ = std::move(cb); }
void StreamingParser::set_on_null(OnNull cb) { on_null_ = std::move(cb); }

bool StreamingParser::feed(const uint8_t* data, size_t size) {
    if (!data || size == 0) return true;
    const char* p = reinterpret_cast<const char*>(data);
    return parse_chunk(p, size);
}

bool StreamingParser::finish() {
    // Try to flush any buffered tokens; for simplicity we treat incomplete
    // trailing data as an error.
    if (!buffer_.empty()) return false;
    return true;
}

bool StreamingParser::parse_chunk(const char* p, size_t n) {
    // Very small streaming implementation: scan for structural tokens and
    // forward simple string/number events. This intentionally tolerates
    // partial tokens across calls by buffering unfinished pieces.
    size_t i = 0;
    while (i < n) {
        char c = p[i++];
        switch (state_) {
            case State::Root:
                if (c == '{') { if (on_start_object_) on_start_object_(); }
                else if (c == '}') { if (on_end_object_) on_end_object_(); }
                else if (c == '[') { if (on_start_array_) on_start_array_(); }
                else if (c == ']') { if (on_end_array_) on_end_array_(); }
                else if (c == '"') { state_ = State::InString; buffer_.clear(); }
                else if ((c >= '0' && c <= '9') || c == '-' || c == '+') {
                    // start number
                    buffer_.clear(); buffer_.push_back(c); state_ = State::InNumber;
                }
                else if (std::isspace((unsigned char)c) || c==',' || c==':') { /* skip */ }
                else if (c == 't') { if (on_bool_) on_bool_(true); }
                else if (c == 'f') { if (on_bool_) on_bool_(false); }
                else if (c == 'n') { if (on_null_) on_null_(); }
                break;
            case State::InString:
                if (c == '\\') { state_ = State::Escaped; }
                else if (c == '"') { state_ = State::Root; if (on_string_) on_string_(buffer_); else buffer_.clear(); }
                else buffer_.push_back(c);
                break;
            case State::Escaped:
                // accept any escaped char
                buffer_.push_back(c);
                state_ = State::InString;
                break;
            case State::InNumber:
                if ( (c >= '0' && c <= '9') || c=='.' || c=='e' || c=='E' || c=='+' || c=='-' ) { buffer_.push_back(c); }
                else {
                    // commit number
                    char* endptr = nullptr;
                    double v = std::strtod(buffer_.c_str(), &endptr);
                    if (on_number_) on_number_(v);
                    buffer_.clear(); state_ = State::Root;
                    // re-evaluate this char in Root state
                    --i;
                }
                break;
        }
    }
    return true;
}
