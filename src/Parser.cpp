#include "Parser.hpp"
#include <iostream>

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

std::vector<std::unique_ptr<AST::Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<AST::Stmt>> statements;
    while (!isAtEnd()) {
        try {
            statements.push_back(declaration());
        } catch (const ParseError& e) {
            synchronize();
        }
    }
    return statements;
}

std::unique_ptr<AST::Stmt> Parser::declaration() {
    if (match({TokenType::INTEGER, TokenType::DECIMAL, TokenType::CHARACTER, TokenType::STRING, TokenType::BOOLEAN, TokenType::ARRAY, TokenType::LET})) {
        TokenType type = previous().type;
        if (check(TokenType::IDENTIFIER) && (peek().type == TokenType::IDENTIFIER || peek().type == TokenType::LEFT_PAREN)) {
             // If next is identifier or '(', it's likely a function or a variable.
             // But if it's '(', it's definitely a function.
             if (peek().type == TokenType::IDENTIFIER && tokens[current+1].type == TokenType::LEFT_PAREN) {
                 return functionDeclaration(type);
             }
        }
        return varDeclaration(type);
    }
    return statement();
}

std::unique_ptr<AST::Stmt> Parser::functionDeclaration(TokenType returnType) {
    Token name = consume(TokenType::IDENTIFIER, "Expect function name.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
    
    std::vector<AST::FunctionStmt::Param> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            TokenType pType = advance().type; // Simplification: assume type keyword
            Token pName = consume(TokenType::IDENTIFIER, "Expect parameter name.");
            parameters.push_back({pType, pName});
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    
    consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
    auto body = block();
    
    return std::make_unique<AST::FunctionStmt>(returnType, name, std::move(parameters), std::move(body));
}

std::unique_ptr<AST::Stmt> Parser::varDeclaration(TokenType type) {
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
    std::unique_ptr<AST::Expr> initializer = nullptr;
    if (match({TokenType::EQUAL})) {
        initializer = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_unique<AST::VarDeclStmt>(type, name, std::move(initializer));
}

std::unique_ptr<AST::Stmt> Parser::statement() {
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::DURING})) return duringStatement();
    if (match({TokenType::FOR})) return forStatement();
    if (match({TokenType::RETURN})) return returnStatement();
    if (match({TokenType::DISPLAY, TokenType::DISPLAYLN})) return displayStatement();
    if (match({TokenType::LEFT_BRACE})) return block();
    return expressionStatement();
}

std::unique_ptr<AST::Stmt> Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    auto condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");
    
    auto thenBranch = statement();
    std::unique_ptr<AST::Stmt> elseBranch = nullptr;
    if (match({TokenType::ELSE})) {
        elseBranch = statement();
    }
    return std::make_unique<AST::IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<AST::Stmt> Parser::duringStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'during'.");
    auto condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    auto body = statement();
    return std::make_unique<AST::DuringStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<AST::Stmt> Parser::forStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");
    
    TokenType varType = TokenType::LET;
    if (match({TokenType::INTEGER, TokenType::DECIMAL, TokenType::LET})) {
        varType = previous().type;
    }
    
    Token varName = consume(TokenType::IDENTIFIER, "Expect loop variable name.");
    consume(TokenType::FROM, "Expect 'from' in for loop.");
    auto startExpr = expression();
    consume(TokenType::TO, "Expect 'to' in for loop.");
    auto endExpr = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for loop header.");
    
    auto body = statement();
    return std::make_unique<AST::ForStmt>(varType, varName, std::move(startExpr), std::move(endExpr), std::move(body));
}

std::unique_ptr<AST::Stmt> Parser::returnStatement() {
    Token keyword = previous();
    std::unique_ptr<AST::Expr> value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_unique<AST::ReturnStmt>(keyword, std::move(value));
}

std::unique_ptr<AST::Stmt> Parser::displayStatement() {
    TokenType type = previous().type;
    consume(TokenType::LEFT_PAREN, "Expect '(' after display function.");
    auto expr = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
    consume(TokenType::SEMICOLON, "Expect ';' after display statement.");
    if (type == TokenType::DISPLAY)
        return std::make_unique<AST::DisplayStmt>(std::move(expr));
    else
        return std::make_unique<AST::DisplayLnStmt>(std::move(expr));
}

std::unique_ptr<AST::BlockStmt> Parser::block() {
    std::vector<std::unique_ptr<AST::Stmt>> statements;
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }
    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return std::make_unique<AST::BlockStmt>(std::move(statements));
}

std::unique_ptr<AST::Stmt> Parser::expressionStatement() {
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<AST::ExprStmt>(std::move(expr));
}

std::unique_ptr<AST::Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<AST::Expr> Parser::assignment() {
    auto expr = logicalOr();
    if (match({TokenType::EQUAL})) {
        Token equals = previous();
        auto value = assignment();
        if (dynamic_cast<AST::VariableExpr*>(expr.get()) || dynamic_cast<AST::ArrayAccess*>(expr.get())) {
            return std::make_unique<AST::BinaryExpr>(std::move(expr), equals, std::move(value));
        }
        throw error(equals, "Invalid assignment target.");
    }
    return expr;
}

std::unique_ptr<AST::Expr> Parser::logicalOr() {
    auto expr = logicalAnd();
    while (match({TokenType::OR})) {
        Token op = previous();
        auto right = logicalAnd();
        expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<AST::Expr> Parser::logicalAnd() {
    auto expr = equality();
    while (match({TokenType::AND})) {
        Token op = previous();
        auto right = equality();
        expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<AST::Expr> Parser::equality() {
    auto expr = comparison();
    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        Token op = previous();
        auto right = comparison();
        expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<AST::Expr> Parser::comparison() {
    auto expr = term();
    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        auto right = term();
        expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<AST::Expr> Parser::term() {
    auto expr = factor();
    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = previous();
        auto right = factor();
        expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<AST::Expr> Parser::factor() {
    auto expr = unary(); // We'll put power here
    while (match({TokenType::SLASH, TokenType::STAR, TokenType::PERCENT, TokenType::CARET})) {
        Token op = previous();
        auto right = unary();
        expr = std::make_unique<AST::BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<AST::Expr> Parser::unary() {
    if (match({TokenType::NOT, TokenType::MINUS})) {
        Token op = previous();
        auto right = unary();
        return std::make_unique<AST::UnaryExpr>(op, std::move(right));
    }
    return call();
}

std::unique_ptr<AST::Expr> Parser::call() {
    auto expr = primary();
    while (true) {
        if (match({TokenType::LEFT_PAREN})) {
            std::vector<std::unique_ptr<AST::Expr>> arguments;
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    arguments.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            Token paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
            if (auto* varExpr = dynamic_cast<AST::VariableExpr*>(expr.get())) {
                expr = std::make_unique<AST::CallExpr>(varExpr->name, std::move(arguments));
            } else {
                throw error(paren, "Can only call functions.");
            }
        } else if (match({TokenType::LEFT_BRACKET})) {
            auto index = expression();
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after index.");
            expr = std::make_unique<AST::ArrayAccess>(std::move(expr), std::move(index));
        } else {
            break;
        }
    }
    return expr;
}

std::unique_ptr<AST::Expr> Parser::primary() {
    if (match({TokenType::BOOLEAN_LIT})) return std::make_unique<AST::BoolLiteral>(previous().lexeme == "true");
    if (match({TokenType::INTEGER_LIT})) return std::make_unique<AST::IntLiteral>(std::stoi(previous().lexeme));
    if (match({TokenType::DECIMAL_LIT})) return std::make_unique<AST::DecimalLiteral>(std::stod(previous().lexeme));
    if (match({TokenType::STRING_LIT})) return std::make_unique<AST::StringLiteral>(previous().lexeme);
    if (match({TokenType::IDENTIFIER}) || match({TokenType::ENTER}) || match({TokenType::ARRAY})) return std::make_unique<AST::VariableExpr>(previous());
    
    if (match({TokenType::LEFT_PAREN})) {
        auto expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }

    if (match({TokenType::LEFT_BRACKET})) {
        return arrayLiteral();
    }
    
    throw error(peek(), "Expect expression.");
}

std::unique_ptr<AST::Expr> Parser::arrayLiteral() {
    std::vector<std::unique_ptr<AST::Expr>> elements;
    if (!check(TokenType::RIGHT_BRACKET)) {
        do {
            elements.push_back(expression());
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_BRACKET, "Expect ']' after array elements.");
    return std::make_unique<AST::ArrayLiteral>(std::move(elements));
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw error(peek(), message);
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        switch (peek().type) {
            case TokenType::INTEGER:
            case TokenType::DECIMAL:
            case TokenType::CHARACTER:
            case TokenType::STRING:
            case TokenType::BOOLEAN:
            case TokenType::ARRAY:
            case TokenType::ARRAY2D:
            case TokenType::LET:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::DURING:
            case TokenType::RETURN:
            case TokenType::DISPLAY:
                return;
            default:
                break;
        }
        advance();
    }
}

ParseError Parser::error(Token token, const std::string& message) {
    std::cerr << "Error at line " << token.line << ", col " << token.column << " ('" << token.lexeme << "'): " << message << std::endl;
    hadError = true;
    return ParseError(message);
}
