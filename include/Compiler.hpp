#pragma once
#include "AST.hpp"
#include "Chunk.hpp"
#include "SymbolTable.hpp"
#include "Function.hpp"
#include <map>

struct CompileResult {
    vm::Chunk mainChunk;
    std::map<std::string, vm::Function> functions;
};

class Compiler : public AST::Visitor {
public:
    Compiler();
    CompileResult compile(const std::vector<std::unique_ptr<AST::Stmt>>& statements);

    void visitIntLiteral(AST::IntLiteral& node) override;
    void visitDecimalLiteral(AST::DecimalLiteral& node) override;
    void visitStringLiteral(AST::StringLiteral& node) override;
    void visitCharLiteral(AST::CharLiteral& node) override;
    void visitBoolLiteral(AST::BoolLiteral& node) override;
    void visitVariableExpr(AST::VariableExpr& node) override;
    void visitBinaryExpr(AST::BinaryExpr& node) override;
    void visitUnaryExpr(AST::UnaryExpr& node) override;
    void visitCallExpr(AST::CallExpr& node) override;
    void visitArrayLiteral(AST::ArrayLiteral& node) override;
    void visitArrayAccess(AST::ArrayAccess& node) override;
    void visitExprStmt(AST::ExprStmt& node) override;
    void visitVarDeclStmt(AST::VarDeclStmt& node) override;
    void visitBlockStmt(AST::BlockStmt& node) override;
    void visitIfStmt(AST::IfStmt& node) override;
    void visitDuringStmt(AST::DuringStmt& node) override;
    void visitForStmt(AST::ForStmt& node) override;
    void visitFunctionStmt(AST::FunctionStmt& node) override;
    void visitReturnStmt(AST::ReturnStmt& node) override;
    void visitDisplayStmt(AST::DisplayStmt& node) override;
    void visitDisplayLnStmt(AST::DisplayLnStmt& node) override;

private:
    vm::Chunk chunk;
    SymbolTable* symbolTable;
    std::map<std::string, vm::Function> compiledFunctions;

    void emitByte(uint8_t byte);
    void emitOp(OpCode op);
    void emitConstant(vm::Value value);
    int emitJump(OpCode instruction);
    void patchJump(int offset);
    void emitLoop(int loopStart);
    
    TokenType lastExprType; // For type checking
};
