#include <iostream>
#include "json_parser.h"
#include "large_data_gen.cpp"

int main_example3() {
    std::string big = generate_large_json(4, 4);
    parse_json_like(reinterpret_cast<const uint8_t*>(big.data()), big.size());
    std::cout << "parsed big size=" << big.size() << "\n";
    return 0;
}
