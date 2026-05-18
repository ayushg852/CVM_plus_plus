#pragma once
#include <string>
#include <stdexcept>
#include <map>
#include <vector>
#include <memory>
#include "TokenType.hpp"

struct Symbol {
    std::string name;
    TokenType type;
    int index; // Index in the local variable array/stack
};

class SymbolTable {
public:
    SymbolTable(SymbolTable* enclosing = nullptr) : enclosing(enclosing) {
        if (enclosing) {
            nextIndex = enclosing->nextIndex;
        }
    }

    void define(const std::string& name, TokenType type) {
        symbols[name] = {name, type, nextIndex++};
        if (enclosing) {
            // This is a bit simplistic, we might need to sync back or just use a pointer to nextIndex
        }
    }

    bool exists(const std::string& name) const {
        if (symbols.find(name) != symbols.end()) return true;
        if (enclosing) return enclosing->exists(name);
        return false;
    }

    Symbol get(const std::string& name) {
        if (symbols.find(name) != symbols.end()) return symbols[name];
        if (enclosing) return enclosing->get(name);
        throw std::runtime_error("Undefined variable '" + name + "'.");
    }

    int size() const { return symbols.size(); }
    int totalSize() const {
        int s = symbols.size();
        if (enclosing) s += enclosing->totalSize();
        return s;
    }

    SymbolTable* getEnclosing() const { return enclosing; }

private:
    std::map<std::string, Symbol> symbols;
    SymbolTable* enclosing;
    int nextIndex = 0;
};
