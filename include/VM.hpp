#pragma once
#include "Chunk.hpp"
#include "Value.hpp"
#include "Function.hpp"
#include <vector>
#include <stack>
#include <map>

namespace vm {

struct CallFrame {
    const Function* function;
    uint32_t ip;
    int stackStart;
};

enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

class VM {
public:
    VM();
    InterpretResult interpret(const Chunk& mainChunk, const std::map<std::string, Function>& functions);

private:
    std::vector<CallFrame> frames;
    std::vector<Value> stack;
    std::vector<Value> globals;
    std::map<std::string, Function> functions;

    InterpretResult run();
    
    void push(Value value);
    Value pop();
    Value peek(int distance = 0);
    
    uint8_t readByte();
    Value readConstant();
    uint16_t readShort();

    void runtimeError(const std::string& message);
};

} // namespace vm
