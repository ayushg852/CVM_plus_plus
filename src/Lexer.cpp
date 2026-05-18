#include "Lexer.hpp"
#include <iostream>

const std::map<std::string, TokenType> Lexer::keywords = {
    {"integer", TokenType::INTEGER},
    {"decimal", TokenType::DECIMAL},
    {"character", TokenType::CHARACTER},
    {"string", TokenType::STRING},
    {"boolean", TokenType::BOOLEAN},
    {"array", TokenType::ARRAY},
    {"array2D", TokenType::ARRAY2D},
    {"let", TokenType::LET},
    {"for", TokenType::FOR},
    {"from", TokenType::FROM},
    {"to", TokenType::TO},
    {"during", TokenType::DURING},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"return", TokenType::RETURN},
    {"AND", TokenType::AND},
    {"OR", TokenType::OR},
    {"NOT", TokenType::NOT},
    {"display", TokenType::DISPLAY},
    {"displayln", TokenType::DISPLAYLN},
    {"enter", TokenType::ENTER},
    {"true", TokenType::BOOLEAN_LIT},
    {"false", TokenType::BOOLEAN_LIT}
};

Lexer::Lexer(std::string source) : source(std::move(source)) {}

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }
    tokens.emplace_back(TokenType::EOF_TOKEN, "", line, column);
    return tokens;
}

bool Lexer::isAtEnd() const {
    return current >= source.length();
}

char Lexer::advance() {
    column++;
    return source[current++];
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() const {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    current++;
    column++;
    return true;
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '[': addToken(TokenType::LEFT_BRACKET); break;
        case ']': addToken(TokenType::RIGHT_BRACKET); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ',': addToken(TokenType::COMMA); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '+': addToken(TokenType::PLUS); break;
        case '-': addToken(TokenType::MINUS); break;
        case '*': addToken(TokenType::STAR); break;
        case '/':
            if (match('/')) {
                // A single-line comment goes until the end of the line.
                while (peek() != '\n' && !isAtEnd()) advance();
            } else if (match('*')) {
                // A multi-line comment goes until '*/'
                while (!isAtEnd()) {
                    if (peek() == '*' && peekNext() == '/') {
                        advance(); // consume '*'
                        advance(); // consume '/'
                        break;
                    }
                    if (peek() == '\n') {
                        line++;
                        column = 1;
                    }
                    advance();
                }
            } else {
                addToken(TokenType::SLASH);
            }
            break;
        case '%': addToken(TokenType::PERCENT); break;
        case '^': addToken(TokenType::CARET); break;
        case '!':
            addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::INVALID);
            break;
        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        case '>':
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;
        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace.
            break;
        case '\n':
            line++;
            column = 1;
            break;
        case '"': string(); break;
        case '\'': charLiteral(); break;
        default:
            if (isDigit(c)) {
                number();
            } else if (isAlpha(c)) {
                identifier();
            } else {
                std::cerr << "Unexpected character at line " << line << ", col " << column << ": " << c << std::endl;
                hadError = true;
            }
            break;
    }
}

void Lexer::addToken(TokenType type) {
    addToken(type, source.substr(start, current - start));
}

void Lexer::addToken(TokenType type, std::string lexeme) {
    tokens.emplace_back(type, std::move(lexeme), line, column - (current - start));
}

void Lexer::identifier() {
    while (isAlphaNumeric(peek())) advance();

    std::string text = source.substr(start, current - start);
    TokenType type = TokenType::IDENTIFIER;
    auto it = keywords.find(text);
    if (it != keywords.end()) type = it->second;
    addToken(type);
}

void Lexer::number() {
    while (isDigit(peek())) advance();

    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext())) {
        // Consume the "."
        advance();
        while (isDigit(peek())) advance();
        addToken(TokenType::DECIMAL_LIT);
    } else {
        addToken(TokenType::INTEGER_LIT);
    }
}

void Lexer::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            line++;
            column = 1;
        }
        advance();
    }

    if (isAtEnd()) {
        std::cerr << "Unterminated string at line " << line << std::endl;
        hadError = true;
        return;
    }

    // The closing ".
    advance();

    // Trim the surrounding quotes.
    std::string value = source.substr(start + 1, current - start - 2);
    addToken(TokenType::STRING_LIT, value);
}

void Lexer::charLiteral() {
    if (peek() != '\'' && !isAtEnd()) {
        advance();
    }
    if (peek() == '\'') {
        advance();
        std::string value = source.substr(start + 1, current - start - 2);
        addToken(TokenType::CHAR_LIT, value);
    } else {
        std::cerr << "Unterminated character literal at line " << line << std::endl;
        hadError = true;
    }
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}
