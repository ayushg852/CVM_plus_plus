#pragma once
#include "Token.hpp"
#include <string>
#include <vector>
#include <map>

class Lexer {
public:
    Lexer(std::string source);
    std::vector<Token> scanTokens();
    bool errorOccurred() const { return hadError; }

private:
    std::string source;
    std::vector<Token> tokens;
    int start = 0;
    int current = 0;
    int line = 1;
    int column = 1;
    bool hadError = false;

    static const std::map<std::string, TokenType> keywords;

    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    void scanToken();
    void addToken(TokenType type);
    void addToken(TokenType type, std::string lexeme);
    void identifier();
    void number();
    void string();
    void charLiteral();
    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;
};
