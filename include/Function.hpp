#pragma once
#include "Chunk.hpp"
#include <string>
#include <vector>

namespace vm {

struct Function {
    std::string name;
    int arity;
    Chunk chunk;
};

} // namespace vm
