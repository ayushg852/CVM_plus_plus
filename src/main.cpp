#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <filesystem>
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Compiler.hpp"
#include "VM.hpp"
#include "ASTPrinter.hpp"
#include "Persistence.hpp"

std::string opcodeToString(OpCode op) {
    static std::map<OpCode, std::string> names = {
        {OpCode::PUSH_CONSTANT, "PUSH_CONSTANT"},
        {OpCode::POP, "POP"},
        {OpCode::LOAD_VAR, "LOAD_VAR"},
        {OpCode::STORE_VAR, "STORE_VAR"},
        {OpCode::ADD, "ADD"},
        {OpCode::SUBTRACT, "SUBTRACT"},
        {OpCode::MULTIPLY, "MULTIPLY"},
        {OpCode::DIVIDE, "DIVIDE"},
        {OpCode::MODULO, "MODULO"},
        {OpCode::POWER, "POWER"},
        {OpCode::EQUAL, "EQUAL"},
        {OpCode::NOT_EQUAL, "NOT_EQUAL"},
        {OpCode::LESS, "LESS"},
        {OpCode::LESS_EQUAL, "LESS_EQUAL"},
        {OpCode::GREATER, "GREATER"},
        {OpCode::GREATER_EQUAL, "GREATER_EQUAL"},
        {OpCode::AND, "AND"},
        {OpCode::OR, "OR"},
        {OpCode::NOT, "NOT"},
        {OpCode::JUMP, "JUMP"},
        {OpCode::JUMP_IF_FALSE, "JUMP_IF_FALSE"},
        {OpCode::LOOP, "LOOP"},
        {OpCode::CALL, "CALL"},
        {OpCode::RETURN, "RETURN"},
        {OpCode::BUILD_ARRAY, "BUILD_ARRAY"},
        {OpCode::INDEX_GET, "INDEX_GET"},
        {OpCode::INDEX_SET, "INDEX_SET"},
        {OpCode::ARRAY_SIZE, "ARRAY_SIZE"},
        {OpCode::NEW_ARRAY, "NEW_ARRAY"},
        {OpCode::DISPLAY, "DISPLAY"},
        {OpCode::DISPLAYLN, "DISPLAYLN"},
        {OpCode::ENTER, "ENTER"},
        {OpCode::HALT, "HALT"}
    };
    return names[op];
}

void dumpBytecode(const vm::Chunk& chunk) {
    std::cout << "--- Bytecode Dump ---" << std::endl;
    for (size_t i = 0; i < chunk.code.size(); ) {
        OpCode op = static_cast<OpCode>(chunk.code[i]);
        std::cout << i << ": " << opcodeToString(op);
        
        i++;
        if (op == OpCode::PUSH_CONSTANT || op == OpCode::CALL) {
            std::cout << " (" << (int)chunk.code[i] << ")";
            i++;
        } else if (op == OpCode::LOAD_VAR || op == OpCode::STORE_VAR || op == OpCode::BUILD_ARRAY) {
            std::cout << " index:" << (int)chunk.code[i];
            i++;
        } else if (op == OpCode::JUMP || op == OpCode::JUMP_IF_FALSE || op == OpCode::LOOP) {
            uint16_t offset = (uint16_t)((chunk.code[i] << 8) | chunk.code[i+1]);
            std::cout << " offset:" << offset;
            i += 2;
        }
        std::cout << std::endl;
    }
    std::cout << "--------------------" << std::endl;
}

void run(const std::string& source, bool debug = false, bool compileOnly = false, const std::string& outPath = "") {
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.scanTokens();
    if (lexer.errorOccurred()) return;

    Parser parser(tokens);
    auto statements = parser.parse();
    if (parser.errorOccurred()) return;

    Compiler compiler;
    CompileResult result = compiler.compile(statements);

    if (debug) {
        dumpBytecode(result.mainChunk);
        for (auto const& [name, func] : result.functions) {
            std::cout << "Function: " << name << std::endl;
            dumpBytecode(func.chunk);
        }
        return; // Dump only
    }

    if (compileOnly) {
        vm::writeBinary(outPath, result.mainChunk, result.functions);
        std::cout << "Bytecode saved to " << outPath << std::endl;
        return;
    }

    vm::VM vm;
    vm.interpret(result.mainChunk, result.functions);
}

void runBinary(const std::string& path) {
    auto res = vm::readBinary(path);
    if (!res.success) {
        std::cerr << "Failed to load binary file '" << path << "'." << std::endl;
        return;
    }
    vm::VM vm;
    vm.interpret(res.mainChunk, res.functions);
}

void runFile(const std::string& path, bool debug = false, bool compileOnly = false) {
    if (std::filesystem::path(path).extension() == ".cvmb") {
        runBinary(path);
        return;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file '" << path << "'." << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    
    std::string outPath = "";
    if (compileOnly) {
        outPath = std::filesystem::path(path).replace_extension(".cvmb").string();
    }

    run(buffer.str(), debug, compileOnly, outPath);
}

void repl() {
    std::string line;
    std::cout << "CVM++ REPL (type 'exit' to quit)" << std::endl;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line) || line == "exit") break;
        if (line.empty()) continue;
        run(line);
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else if (argc == 3) {
        std::string arg1 = argv[1];
        if (arg1 == "--debug" || arg1 == "-d") {
            runFile(argv[2], true);
        } else if (arg1 == "--compile" || arg1 == "-c") {
            runFile(argv[2], false, true);
        } else {
            std::cerr << "Unknown option '" << arg1 << "'." << std::endl;
            return 1;
        }
    } else {
        std::cerr << "Usage: cvm [options] [path]" << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "  -c, --compile    Compile source to .cvmb" << std::endl;
        std::cerr << "  -d, --debug      Dump bytecode and exit" << std::endl;
        return 1;
    }

    return 0;
}
