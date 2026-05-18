#pragma once
#include <vector>
#include <cstdint>
#include "OpCode.hpp"
#include "Value.hpp"

namespace vm {

struct Chunk {
    std::vector<uint8_t> code;
    std::vector<Value> constants;

    void write(uint8_t byte) {
        code.push_back(byte);
    }

    void write(OpCode op) {
        code.push_back(static_cast<uint8_t>(op));
    }

    int addConstant(Value value) {
        constants.push_back(value);
        return constants.size() - 1;
    }
};

} // namespace vm
