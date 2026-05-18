#pragma once
#include "Token.hpp"
#include "AST.hpp"
#include <vector>
#include <memory>
#include <stdexcept>

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& message) : std::runtime_error(message) {}
};

class Parser {
public:
    Parser(std::vector<Token> tokens);
    std::vector<std::unique_ptr<AST::Stmt>> parse();
    bool errorOccurred() const { return hadError; }

private:
    std::vector<Token> tokens;
    int current = 0;
    bool hadError = false;

    std::unique_ptr<AST::Stmt> declaration();
    std::unique_ptr<AST::Stmt> functionDeclaration(TokenType returnType);
    std::unique_ptr<AST::Stmt> varDeclaration(TokenType type);
    std::unique_ptr<AST::Stmt> statement();
    std::unique_ptr<AST::Stmt> ifStatement();
    std::unique_ptr<AST::Stmt> duringStatement();
    std::unique_ptr<AST::Stmt> forStatement();
    std::unique_ptr<AST::Stmt> returnStatement();
    std::unique_ptr<AST::Stmt> displayStatement();
    std::unique_ptr<AST::BlockStmt> block();
    std::unique_ptr<AST::Stmt> expressionStatement();

    std::unique_ptr<AST::Expr> expression();
    std::unique_ptr<AST::Expr> assignment();
    std::unique_ptr<AST::Expr> logicalOr();
    std::unique_ptr<AST::Expr> logicalAnd();
    std::unique_ptr<AST::Expr> equality();
    std::unique_ptr<AST::Expr> comparison();
    std::unique_ptr<AST::Expr> term();
    std::unique_ptr<AST::Expr> factor();
    std::unique_ptr<AST::Expr> unary();
    std::unique_ptr<AST::Expr> call();
    std::unique_ptr<AST::Expr> primary();
    std::unique_ptr<AST::Expr> arrayLiteral();

    bool match(const std::vector<TokenType>& types);
    bool check(TokenType type) const;
    Token advance();
    bool isAtEnd() const;
    Token peek() const;
    Token previous() const;
    Token consume(TokenType type, const std::string& message);
    void synchronize();
    ParseError error(Token token, const std::string& message);
};
