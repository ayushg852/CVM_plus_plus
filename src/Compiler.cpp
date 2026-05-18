#include "Compiler.hpp"
#include <iostream>

Compiler::Compiler() {
    symbolTable = new SymbolTable();
}

CompileResult Compiler::compile(const std::vector<std::unique_ptr<AST::Stmt>>& statements) {
    for (const auto& stmt : statements) {
        stmt->accept(*this);
    }
    emitOp(OpCode::HALT);
    return {chunk, compiledFunctions};
}

void Compiler::emitByte(uint8_t byte) {
    chunk.write(byte);
}

void Compiler::emitOp(OpCode op) {
    chunk.write(op);
}

void Compiler::emitConstant(vm::Value value) {
    int index = chunk.addConstant(value);
    emitOp(OpCode::PUSH_CONSTANT);
    emitByte(static_cast<uint8_t>(index));
}

int Compiler::emitJump(OpCode instruction) {
    emitOp(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return chunk.code.size() - 2;
}

void Compiler::patchJump(int offset) {
    int jump = chunk.code.size() - offset - 2;
    if (jump > 0xffff) {
        std::cerr << "Too much code to jump over." << std::endl;
    }
    chunk.code[offset] = (jump >> 8) & 0xff;
    chunk.code[offset + 1] = jump & 0xff;
}

void Compiler::emitLoop(int loopStart) {
    emitOp(OpCode::LOOP);
    int offset = chunk.code.size() - loopStart + 2;
    if (offset > 0xffff) std::cerr << "Loop body too large." << std::endl;
    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

void Compiler::visitIntLiteral(AST::IntLiteral& node) {
    emitConstant(node.value);
    lastExprType = TokenType::INTEGER;
}

void Compiler::visitDecimalLiteral(AST::DecimalLiteral& node) {
    emitConstant(node.value);
    lastExprType = TokenType::DECIMAL;
}

void Compiler::visitStringLiteral(AST::StringLiteral& node) {
    emitConstant(node.value);
    lastExprType = TokenType::STRING;
}

void Compiler::visitCharLiteral(AST::CharLiteral& node) {
    emitConstant(node.value);
    lastExprType = TokenType::CHARACTER;
}

void Compiler::visitBoolLiteral(AST::BoolLiteral& node) {
    emitConstant(node.value);
    lastExprType = TokenType::BOOLEAN;
}

void Compiler::visitVariableExpr(AST::VariableExpr& node) {
    if (symbolTable->exists(node.name.lexeme)) {
        Symbol sym = symbolTable->get(node.name.lexeme);
        emitOp(OpCode::LOAD_VAR);
        emitByte(static_cast<uint8_t>(sym.index));
        lastExprType = sym.type;
    } else {
        std::cerr << "Undefined variable '" << node.name.lexeme << "' at line " << node.name.line << std::endl;
    }
}

void Compiler::visitBinaryExpr(AST::BinaryExpr& node) {
    if (node.op.type == TokenType::EQUAL) {
        if (auto* varExpr = dynamic_cast<AST::VariableExpr*>(node.left.get())) {
            node.right->accept(*this);
            Symbol sym = symbolTable->get(varExpr->name.lexeme);
            emitOp(OpCode::STORE_VAR);
            emitByte(static_cast<uint8_t>(sym.index));
            return;
        } else if (auto* arrayAccess = dynamic_cast<AST::ArrayAccess*>(node.left.get())) {
            // arr[idx] = val
            arrayAccess->array->accept(*this);
            arrayAccess->index->accept(*this);
            node.right->accept(*this);
            emitOp(OpCode::INDEX_SET);
            return;
        }
    }
    
    node.left->accept(*this);
    TokenType leftType = lastExprType;
    node.right->accept(*this);
    TokenType rightType = lastExprType;

    switch (node.op.type) {
        case TokenType::PLUS: emitOp(OpCode::ADD); break;
        case TokenType::MINUS: emitOp(OpCode::SUBTRACT); break;
        case TokenType::STAR: emitOp(OpCode::MULTIPLY); break;
        case TokenType::SLASH: emitOp(OpCode::DIVIDE); break;
        case TokenType::PERCENT: emitOp(OpCode::MODULO); break;
        case TokenType::CARET: emitOp(OpCode::POWER); break;
        case TokenType::EQUAL_EQUAL: emitOp(OpCode::EQUAL); break;
        case TokenType::BANG_EQUAL: emitOp(OpCode::NOT_EQUAL); break;
        case TokenType::LESS: emitOp(OpCode::LESS); break;
        case TokenType::LESS_EQUAL: emitOp(OpCode::LESS_EQUAL); break;
        case TokenType::GREATER: emitOp(OpCode::GREATER); break;
        case TokenType::GREATER_EQUAL: emitOp(OpCode::GREATER_EQUAL); break;
        case TokenType::AND: emitOp(OpCode::AND); break;
        case TokenType::OR: emitOp(OpCode::OR); break;
        default: break;
    }
}

void Compiler::visitUnaryExpr(AST::UnaryExpr& node) {
    node.right->accept(*this);
    if (node.op.type == TokenType::NOT) emitOp(OpCode::NOT);
    else if (node.op.type == TokenType::MINUS) {
        emitConstant(-1);
        emitOp(OpCode::MULTIPLY);
    }
}

void Compiler::visitCallExpr(AST::CallExpr& node) {
    if (node.callee.lexeme == "enter") {
        emitOp(OpCode::ENTER);
        lastExprType = TokenType::LET; 
        return;
    }
    if (node.callee.lexeme == "size") {
        node.arguments[0]->accept(*this);
        emitOp(OpCode::ARRAY_SIZE);
        lastExprType = TokenType::INTEGER;
        return;
    }
    if (node.callee.lexeme == "array") {
        node.arguments[0]->accept(*this);
        emitOp(OpCode::NEW_ARRAY);
        lastExprType = TokenType::ARRAY;
        return;
    }
    for (auto& arg : node.arguments) {
        arg->accept(*this);
    }
    emitOp(OpCode::CALL);
    int index = chunk.addConstant(node.callee.lexeme);
    emitByte(static_cast<uint8_t>(index));
}

void Compiler::visitArrayLiteral(AST::ArrayLiteral& node) {
    for (auto& element : node.elements) {
        element->accept(*this);
    }
    emitOp(OpCode::BUILD_ARRAY);
    emitByte(static_cast<uint8_t>(node.elements.size()));
    lastExprType = TokenType::ARRAY;
}

void Compiler::visitArrayAccess(AST::ArrayAccess& node) {
    node.array->accept(*this);
    node.index->accept(*this);
    emitOp(OpCode::INDEX_GET);
    lastExprType = TokenType::LET; // We don't know element type easily
}

void Compiler::visitExprStmt(AST::ExprStmt& node) {
    node.expression->accept(*this);
    emitOp(OpCode::POP);
}

void Compiler::visitVarDeclStmt(AST::VarDeclStmt& node) {
    if (node.initializer) {
        node.initializer->accept(*this);
    } else {
        emitConstant(0);
        lastExprType = TokenType::INTEGER;
    }

    TokenType finalType = node.type;
    if (finalType == TokenType::LET) {
        if (lastExprType == TokenType::LET) finalType = TokenType::INTEGER;
        else finalType = lastExprType;
    }

    symbolTable->define(node.name.lexeme, finalType);
    Symbol sym = symbolTable->get(node.name.lexeme);
    emitOp(OpCode::STORE_VAR);
    emitByte(static_cast<uint8_t>(sym.index));
}

void Compiler::visitBlockStmt(AST::BlockStmt& node) {
    SymbolTable* previous = symbolTable;
    symbolTable = new SymbolTable(previous);
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
    SymbolTable* current = symbolTable;
    symbolTable = previous;
    delete current;
}

void Compiler::visitIfStmt(AST::IfStmt& node) {
    node.condition->accept(*this);
    int thenJump = emitJump(OpCode::JUMP_IF_FALSE);
    node.thenBranch->accept(*this);
    int elseJump = emitJump(OpCode::JUMP);
    patchJump(thenJump);
    if (node.elseBranch) node.elseBranch->accept(*this);
    patchJump(elseJump);
}

void Compiler::visitDuringStmt(AST::DuringStmt& node) {
    int loopStart = chunk.code.size();
    node.condition->accept(*this);
    int exitJump = emitJump(OpCode::JUMP_IF_FALSE);
    node.body->accept(*this);
    emitLoop(loopStart);
    patchJump(exitJump);
}

void Compiler::visitForStmt(AST::ForStmt& node) {
    SymbolTable* previous = symbolTable;
    symbolTable = new SymbolTable(previous);
    node.startExpr->accept(*this);
    symbolTable->define(node.varName.lexeme, node.varType == TokenType::LET ? lastExprType : node.varType);
    Symbol sym = symbolTable->get(node.varName.lexeme);
    emitOp(OpCode::STORE_VAR);
    emitByte(static_cast<uint8_t>(sym.index));
    int loopStart = chunk.code.size();
    emitOp(OpCode::LOAD_VAR);
    emitByte(static_cast<uint8_t>(sym.index));
    node.endExpr->accept(*this);
    emitOp(OpCode::LESS_EQUAL);
    int exitJump = emitJump(OpCode::JUMP_IF_FALSE);
    node.body->accept(*this);
    emitOp(OpCode::LOAD_VAR);
    emitByte(static_cast<uint8_t>(sym.index));
    emitConstant(1);
    emitOp(OpCode::ADD);
    emitOp(OpCode::STORE_VAR);
    emitByte(static_cast<uint8_t>(sym.index));
    emitLoop(loopStart);
    patchJump(exitJump);
    SymbolTable* current = symbolTable;
    symbolTable = previous;
    delete current;
}

void Compiler::visitFunctionStmt(AST::FunctionStmt& node) {
    Compiler funcCompiler;
    for (const auto& param : node.params) {
        funcCompiler.symbolTable->define(param.name.lexeme, param.type);
    }
    
    bool hasReturn = false;
    for (const auto& stmt : node.body->statements) {
        if (dynamic_cast<AST::ReturnStmt*>(stmt.get())) hasReturn = true;
        stmt.get()->accept(funcCompiler);
    }
    
    if (!hasReturn) {
        std::cerr << "missing return statement at line " << node.name.line << std::endl;
    }
    
    funcCompiler.emitOp(OpCode::HALT);
    
    vm::Function func;
    func.name = node.name.lexeme;
    func.arity = node.params.size();
    func.chunk = funcCompiler.chunk;
    
    compiledFunctions[node.name.lexeme] = func;
}

void Compiler::visitReturnStmt(AST::ReturnStmt& node) {
    if (node.value) node.value->accept(*this);
    else emitConstant(0);
    emitOp(OpCode::RETURN);
}

void Compiler::visitDisplayStmt(AST::DisplayStmt& node) {
    node.expression->accept(*this);
    emitOp(OpCode::DISPLAY);
}

void Compiler::visitDisplayLnStmt(AST::DisplayLnStmt& node) {
    node.expression->accept(*this);
    emitOp(OpCode::DISPLAYLN);
}
