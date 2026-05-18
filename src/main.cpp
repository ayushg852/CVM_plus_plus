#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Compiler.hpp"
#include "VM.hpp"

void dumpBytecode(const vm::Chunk& chunk) {
    std::cout << "--- Bytecode Dump ---" << std::endl;
    for (size_t i = 0; i < chunk.code.size(); ++i) {
        std::cout << i << ": " << (int)chunk.code[i] << std::endl;
    }
    std::cout << "--------------------" << std::endl;
}

void run(const std::string& source, bool debug = false) {
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
    }

    vm::VM vm;
    vm.interpret(result.mainChunk, result.functions);
}

void runFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file '" << path << "'." << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    run(buffer.str());
}

void repl() {
    std::string line;
    std::cout << "CVM++ REPL (type 'exit' to quit)" << std::endl;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line) || line == "exit") break;
        if (line.empty()) continue;
        
        // REPL might be tricky with statements vs expressions
        // For now, just run it.
        run(line);
    }
}

int main(int argc, char* argv[]) {
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        std::cerr << "Usage: cvm [path]" << std::endl;
        return 1;
    }

    return 0;
}
