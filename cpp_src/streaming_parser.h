#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

// Streaming parser callbacks for incremental parsing.
// The parser processes chunks of bytes and invokes callbacks for structural events.
class StreamingParser {
public:
    using OnStartObject = std::function<void()>;
    using OnEndObject = std::function<void()>;
    using OnStartArray = std::function<void()>;
    using OnEndArray = std::function<void()>;
    using OnKey = std::function<void(const std::string&)>;
    using OnString = std::function<void(const std::string&)>;
    using OnNumber = std::function<void(double)>;
    using OnBool = std::function<void(bool)>;
    using OnNull = std::function<void()>;

    StreamingParser();
    void set_on_start_object(OnStartObject cb);
    void set_on_end_object(OnEndObject cb);
    void set_on_start_array(OnStartArray cb);
    void set_on_end_array(OnEndArray cb);
    void set_on_key(OnKey cb);
    void set_on_string(OnString cb);
    void set_on_number(OnNumber cb);
    void set_on_bool(OnBool cb);
    void set_on_null(OnNull cb);

    // Feed a chunk of data; returns false if a fatal parse error occurred.
    bool feed(const uint8_t* data, size_t size);
    // Signal end-of-input; returns true if parsing ended cleanly.
    bool finish();

private:
    enum class State { Root, InString, InNumber, Escaped };
    State state_;
    std::string buffer_;
    // callbacks
    OnStartObject on_start_object_;
    OnEndObject on_end_object_;
    OnStartArray on_start_array_;
    OnEndArray on_end_array_;
    OnKey on_key_;
    OnString on_string_;
    OnNumber on_number_;
    OnBool on_bool_;
    OnNull on_null_;
    bool parse_chunk(const char* p, size_t n);
};
