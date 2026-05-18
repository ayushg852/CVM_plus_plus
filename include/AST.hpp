#pragma once
#include "Token.hpp"
#include <vector>
#include <memory>
#include <string>

namespace AST {

class Visitor;

class Node {
public:
    virtual ~Node() = default;
    virtual void accept(Visitor& visitor) = 0;
};

class Expr : public Node {};
class Stmt : public Node {};

// --- Expressions ---

class IntLiteral : public Expr {
public:
    int value;
    IntLiteral(int val) : value(val) {}
    void accept(Visitor& visitor) override;
};

class DecimalLiteral : public Expr {
public:
    double value;
    DecimalLiteral(double val) : value(val) {}
    void accept(Visitor& visitor) override;
};

class StringLiteral : public Expr {
public:
    std::string value;
    StringLiteral(std::string val) : value(std::move(val)) {}
    void accept(Visitor& visitor) override;
};

class BoolLiteral : public Expr {
public:
    bool value;
    BoolLiteral(bool val) : value(val) {}
    void accept(Visitor& visitor) override;
};

class VariableExpr : public Expr {
public:
    Token name;
    VariableExpr(Token name) : name(std::move(name)) {}
    void accept(Visitor& visitor) override;
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}
    void accept(Visitor& visitor) override;
};

class UnaryExpr : public Expr {
public:
    Token op;
    std::unique_ptr<Expr> right;
    UnaryExpr(Token op, std::unique_ptr<Expr> right)
        : op(std::move(op)), right(std::move(right)) {}
    void accept(Visitor& visitor) override;
};

class CallExpr : public Expr {
public:
    Token callee;
    std::vector<std::unique_ptr<Expr>> arguments;
    CallExpr(Token callee, std::vector<std::unique_ptr<Expr>> args)
        : callee(std::move(callee)), arguments(std::move(args)) {}
    void accept(Visitor& visitor) override;
};

class ArrayLiteral : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;
    ArrayLiteral(std::vector<std::unique_ptr<Expr>> elems) : elements(std::move(elems)) {}
    void accept(Visitor& visitor) override;
};

class ArrayAccess : public Expr {
public:
    std::unique_ptr<Expr> array;
    std::unique_ptr<Expr> index;
    ArrayAccess(std::unique_ptr<Expr> arr, std::unique_ptr<Expr> idx)
        : array(std::move(arr)), index(std::move(idx)) {}
    void accept(Visitor& visitor) override;
};

// --- Statements ---

class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    ExprStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
    void accept(Visitor& visitor) override;
};

class VarDeclStmt : public Stmt {
public:
    TokenType type; // INTEGER, DECIMAL, STRING, BOOLEAN, CHARACTER, LET
    Token name;
    std::unique_ptr<Expr> initializer;
    VarDeclStmt(TokenType type, Token name, std::unique_ptr<Expr> init)
        : type(type), name(std::move(name)), initializer(std::move(init)) {}
    void accept(Visitor& visitor) override;
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts) : statements(std::move(stmts)) {}
    void accept(Visitor& visitor) override;
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
    IfStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> thenB, std::unique_ptr<Stmt> elseB)
        : condition(std::move(cond)), thenBranch(std::move(thenB)), elseBranch(std::move(elseB)) {}
    void accept(Visitor& visitor) override;
};

class DuringStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    DuringStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> body)
        : condition(std::move(cond)), body(std::move(body)) {}
    void accept(Visitor& visitor) override;
};

class ForStmt : public Stmt {
public:
    TokenType varType;
    Token varName;
    std::unique_ptr<Expr> startExpr;
    std::unique_ptr<Expr> endExpr;
    std::unique_ptr<Stmt> body;
    ForStmt(TokenType type, Token name, std::unique_ptr<Expr> start, std::unique_ptr<Expr> end, std::unique_ptr<Stmt> body)
        : varType(type), varName(std::move(name)), startExpr(std::move(start)), endExpr(std::move(end)), body(std::move(body)) {}
    void accept(Visitor& visitor) override;
};

class FunctionStmt : public Stmt {
public:
    TokenType returnType;
    Token name;
    struct Param { TokenType type; Token name; };
    std::vector<Param> params;
    std::unique_ptr<BlockStmt> body;
    FunctionStmt(TokenType retType, Token name, std::vector<Param> params, std::unique_ptr<BlockStmt> body)
        : returnType(retType), name(std::move(name)), params(std::move(params)), body(std::move(body)) {}
    void accept(Visitor& visitor) override;
};

class ReturnStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> value;
    ReturnStmt(Token keyword, std::unique_ptr<Expr> val) : keyword(std::move(keyword)), value(std::move(val)) {}
    void accept(Visitor& visitor) override;
};

class DisplayStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    DisplayStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
    void accept(Visitor& visitor) override;
};

class DisplayLnStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    DisplayLnStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
    void accept(Visitor& visitor) override;
};

// --- Visitor ---

class Visitor {
public:
    virtual ~Visitor() = default;
    virtual void visitIntLiteral(IntLiteral& node) = 0;
    virtual void visitDecimalLiteral(DecimalLiteral& node) = 0;
    virtual void visitStringLiteral(StringLiteral& node) = 0;
    virtual void visitBoolLiteral(BoolLiteral& node) = 0;
    virtual void visitVariableExpr(VariableExpr& node) = 0;
    virtual void visitBinaryExpr(BinaryExpr& node) = 0;
    virtual void visitUnaryExpr(UnaryExpr& node) = 0;
    virtual void visitCallExpr(CallExpr& node) = 0;
    virtual void visitArrayLiteral(ArrayLiteral& node) = 0;
    virtual void visitArrayAccess(ArrayAccess& node) = 0;
    virtual void visitExprStmt(ExprStmt& node) = 0;
    virtual void visitVarDeclStmt(VarDeclStmt& node) = 0;
    virtual void visitBlockStmt(BlockStmt& node) = 0;
    virtual void visitIfStmt(IfStmt& node) = 0;
    virtual void visitDuringStmt(DuringStmt& node) = 0;
    virtual void visitForStmt(ForStmt& node) = 0;
    virtual void visitFunctionStmt(FunctionStmt& node) = 0;
    virtual void visitReturnStmt(ReturnStmt& node) = 0;
    virtual void visitDisplayStmt(DisplayStmt& node) = 0;
    virtual void visitDisplayLnStmt(DisplayLnStmt& node) = 0;
};

inline void IntLiteral::accept(Visitor& v) { v.visitIntLiteral(*this); }
inline void DecimalLiteral::accept(Visitor& v) { v.visitDecimalLiteral(*this); }
inline void StringLiteral::accept(Visitor& v) { v.visitStringLiteral(*this); }
inline void BoolLiteral::accept(Visitor& v) { v.visitBoolLiteral(*this); }
inline void VariableExpr::accept(Visitor& v) { v.visitVariableExpr(*this); }
inline void BinaryExpr::accept(Visitor& v) { v.visitBinaryExpr(*this); }
inline void UnaryExpr::accept(Visitor& v) { v.visitUnaryExpr(*this); }
inline void CallExpr::accept(Visitor& v) { v.visitCallExpr(*this); }
inline void ArrayLiteral::accept(Visitor& v) { v.visitArrayLiteral(*this); }
inline void ArrayAccess::accept(Visitor& v) { v.visitArrayAccess(*this); }
inline void ExprStmt::accept(Visitor& v) { v.visitExprStmt(*this); }
inline void VarDeclStmt::accept(Visitor& v) { v.visitVarDeclStmt(*this); }
inline void BlockStmt::accept(Visitor& v) { v.visitBlockStmt(*this); }
inline void IfStmt::accept(Visitor& v) { v.visitIfStmt(*this); }
inline void DuringStmt::accept(Visitor& v) { v.visitDuringStmt(*this); }
inline void ForStmt::accept(Visitor& v) { v.visitForStmt(*this); }
inline void FunctionStmt::accept(Visitor& v) { v.visitFunctionStmt(*this); }
inline void ReturnStmt::accept(Visitor& v) { v.visitReturnStmt(*this); }
inline void DisplayStmt::accept(Visitor& v) { v.visitDisplayStmt(*this); }
inline void DisplayLnStmt::accept(Visitor& v) { v.visitDisplayLnStmt(*this); }

} // namespace AST
