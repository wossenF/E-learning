#include <iostream>
#include <string>
#include "json_parser.h"
#include "serializer.h"
#include "codec.h"

// Larger example that composes multiple substrate pieces into a demo
void run_demo() {
    const char* sample = "{\"users\":[{\"id\":1,\"name\":\"alice\"},{\"id\":2,\"name\":\"bob\"}],\"meta\":{\"count\":2}}";
    parse_json_like(reinterpret_cast<const uint8_t*>(sample), strlen(sample));
    // pretend to create a JsonValue and serialize it (not parsing here to keep code small)
    std::string encoded = base64_encode(std::vector<uint8_t>{'o','k'});
    std::cout << "demo encoded: " << encoded << "\n";
}

int main_demo() { run_demo(); return 0; }
