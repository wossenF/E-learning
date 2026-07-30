#include "json_parser.h"
#include <iostream>

int main() {
    const char* sample = "{\"hello\": [\"world\", 123, {\"nested\": true}]}";
    bool ok = parse_json_like(reinterpret_cast<const uint8_t*>(sample), strlen(sample));
    std::cout << "parse ok=" << ok << "\n";
    return ok ? 0 : 2;
}
