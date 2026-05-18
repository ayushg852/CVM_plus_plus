#pragma once
#include <cstdint>

enum class OpCode : uint8_t {
    PUSH_CONSTANT,
    POP,
    
    LOAD_VAR,
    STORE_VAR,
    
    ADD,
    SUBTRACT,
    MULTIPLY,
    DIVIDE,
    MODULO,
    POWER,
    
    EQUAL,
    NOT_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    
    AND,
    OR,
    NOT,
    
    JUMP,
    JUMP_IF_FALSE,
    LOOP,
    
    CALL,
    RETURN,
    
    BUILD_ARRAY,
    INDEX_GET,
    INDEX_SET,
    ARRAY_SIZE,
    NEW_ARRAY,
    
    DISPLAY,
    DISPLAYLN,
    ENTER,
    
    HALT
};
