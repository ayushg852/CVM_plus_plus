#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include "Chunk.hpp"
#include "Function.hpp"
#include "Value.hpp"

namespace vm {

enum class ValueType : uint8_t {
    INT = 0,
    DOUBLE = 1,
    CHAR = 2,
    STRING = 3,
    BOOL = 4,
    ARRAY = 5,
    NIL = 6
};

inline void writeValue(std::ostream& os, const Value& v) {
    if (std::holds_alternative<int>(v)) {
        os.put(static_cast<char>(ValueType::INT));
        int val = std::get<int>(v);
        os.write(reinterpret_cast<const char*>(&val), sizeof(int));
    } else if (std::holds_alternative<double>(v)) {
        os.put(static_cast<char>(ValueType::DOUBLE));
        double val = std::get<double>(v);
        os.write(reinterpret_cast<const char*>(&val), sizeof(double));
    } else if (std::holds_alternative<char>(v)) {
        os.put(static_cast<char>(ValueType::CHAR));
        char val = std::get<char>(v);
        os.put(val);
    } else if (std::holds_alternative<std::string>(v)) {
        os.put(static_cast<char>(ValueType::STRING));
        const std::string& s = std::get<std::string>(v);
        uint32_t size = static_cast<uint32_t>(s.size());
        os.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
        os.write(s.data(), size);
    } else if (std::holds_alternative<bool>(v)) {
        os.put(static_cast<char>(ValueType::BOOL));
        bool val = std::get<bool>(v);
        os.put(val ? 1 : 0);
    } else {
        os.put(static_cast<char>(ValueType::NIL));
    }
}

inline Value readValue(std::istream& is) {
    ValueType type = static_cast<ValueType>(is.get());
    switch (type) {
        case ValueType::INT: {
            int val;
            is.read(reinterpret_cast<char*>(&val), sizeof(int));
            return Value(val);
        }
        case ValueType::DOUBLE: {
            double val;
            is.read(reinterpret_cast<char*>(&val), sizeof(double));
            return Value(val);
        }
        case ValueType::CHAR: {
            return Value(static_cast<char>(is.get()));
        }
        case ValueType::STRING: {
            uint32_t size;
            is.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
            std::string s(size, '\0');
            is.read(&s[0], size);
            return Value(s);
        }
        case ValueType::BOOL: {
            return Value(is.get() != 0);
        }
        default: return Value(0);
    }
}

inline void writeChunk(std::ostream& os, const Chunk& chunk) {
    uint32_t codeSize = static_cast<uint32_t>(chunk.code.size());
    os.write(reinterpret_cast<const char*>(&codeSize), sizeof(uint32_t));
    os.write(reinterpret_cast<const char*>(chunk.code.data()), codeSize);

    uint32_t constSize = static_cast<uint32_t>(chunk.constants.size());
    os.write(reinterpret_cast<const char*>(&constSize), sizeof(uint32_t));
    for (const auto& v : chunk.constants) {
        writeValue(os, v);
    }
}

inline Chunk readChunk(std::istream& is) {
    Chunk chunk;
    uint32_t codeSize;
    is.read(reinterpret_cast<char*>(&codeSize), sizeof(uint32_t));
    chunk.code.resize(codeSize);
    is.read(reinterpret_cast<char*>(chunk.code.data()), codeSize);

    uint32_t constSize;
    is.read(reinterpret_cast<char*>(&constSize), sizeof(uint32_t));
    for (uint32_t i = 0; i < constSize; ++i) {
        chunk.constants.push_back(readValue(is));
    }
    return chunk;
}

inline void writeBinary(const std::string& path, const Chunk& mainChunk, const std::map<std::string, Function>& functions) {
    std::ofstream os(path, std::ios::binary);
    if (!os) return;

    os.write("CVMB", 4); // Magic

    writeChunk(os, mainChunk);

    uint32_t funcCount = static_cast<uint32_t>(functions.size());
    os.write(reinterpret_cast<const char*>(&funcCount), sizeof(uint32_t));
    for (const auto& [name, func] : functions) {
        uint32_t nameSize = static_cast<uint32_t>(name.size());
        os.write(reinterpret_cast<const char*>(&nameSize), sizeof(uint32_t));
        os.write(name.data(), nameSize);
        os.write(reinterpret_cast<const char*>(&func.arity), sizeof(int));
        writeChunk(os, func.chunk);
    }
}

struct LoadResult {
    Chunk mainChunk;
    std::map<std::string, Function> functions;
    bool success = false;
};

inline LoadResult readBinary(const std::string& path) {
    LoadResult res;
    std::ifstream is(path, std::ios::binary);
    if (!is) return res;

    char magic[4];
    is.read(magic, 4);
    if (std::string(magic, 4) != "CVMB") return res;

    res.mainChunk = readChunk(is);

    uint32_t funcCount;
    is.read(reinterpret_cast<char*>(&funcCount), sizeof(uint32_t));
    for (uint32_t i = 0; i < funcCount; ++i) {
        uint32_t nameSize;
        is.read(reinterpret_cast<char*>(&nameSize), sizeof(uint32_t));
        std::string name(nameSize, '\0');
        is.read(&name[0], nameSize);
        
        Function func;
        func.name = name;
        is.read(reinterpret_cast<char*>(&func.arity), sizeof(int));
        func.chunk = readChunk(is);
        
        res.functions[name] = std::move(func);
    }

    res.success = true;
    return res;
}

} // namespace vm
