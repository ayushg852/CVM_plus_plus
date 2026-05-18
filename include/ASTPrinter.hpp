#pragma once
#include "AST.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace AST {

class ASTPrinter : public Visitor {
public:
    void print(const std::vector<std::unique_ptr<Stmt>>& statements) {
        std::cout << "--- AST Tree Dump ---" << std::endl;
        std::cout << "Program" << std::endl;
        for (size_t i = 0; i < statements.size(); ++i) {
            bool isLast = (i == statements.size() - 1);
            printNode(statements[i].get(), "", isLast);
        }
        std::cout << "--------------------" << std::endl;
    }

private:
    void printNode(Node* node, const std::string& prefix, bool isLast, const std::string& label = "") {
        if (!node) return;

        std::cout << prefix;
        std::cout << (isLast ? "`- " : "|- ");
        if (!label.empty()) std::cout << "[" << label << "] ";
        
        std::string oldPrefix = currentPrefix;
        currentPrefix = prefix + (isLast ? "   " : "|  ");
        node->accept(*this);
        std::cout << std::endl;
        currentPrefix = oldPrefix;
    }

    std::string currentPrefix;

    void visitIntLiteral(IntLiteral& node) override {
        std::cout << "IntLiteral: " << node.value;
    }

    void visitDecimalLiteral(DecimalLiteral& node) override {
        std::cout << "DecimalLiteral: " << node.value;
    }

    void visitStringLiteral(StringLiteral& node) override {
        std::cout << "StringLiteral: \"" << node.value << "\"";
    }

    void visitCharLiteral(CharLiteral& node) override {
        std::cout << "CharLiteral: '" << node.value << "'";
    }

    void visitBoolLiteral(BoolLiteral& node) override {
        std::cout << "BoolLiteral: " << (node.value ? "true" : "false");
    }

    void visitVariableExpr(VariableExpr& node) override {
        std::cout << "VariableExpr: " << node.name.lexeme;
    }

    void visitBinaryExpr(BinaryExpr& node) override {
        std::cout << "BinaryExpr: " << node.op.lexeme;
        printNode(node.left.get(), currentPrefix, false, "Left");
        printNode(node.right.get(), currentPrefix, true, "Right");
    }

    void visitUnaryExpr(UnaryExpr& node) override {
        std::cout << "UnaryExpr: " << node.op.lexeme;
        printNode(node.right.get(), currentPrefix, true);
    }

    void visitCallExpr(CallExpr& node) override {
        std::cout << "CallExpr: " << node.callee.lexeme;
        for (size_t i = 0; i < node.arguments.size(); ++i) {
            printNode(node.arguments[i].get(), currentPrefix, i == node.arguments.size() - 1, "Arg " + std::to_string(i));
        }
    }

    void visitArrayLiteral(ArrayLiteral& node) override {
        std::cout << "ArrayLiteral";
        for (size_t i = 0; i < node.elements.size(); ++i) {
            printNode(node.elements[i].get(), currentPrefix, i == node.elements.size() - 1, "Elem " + std::to_string(i));
        }
    }

    void visitArrayAccess(ArrayAccess& node) override {
        std::cout << "ArrayAccess";
        printNode(node.array.get(), currentPrefix, false, "Array");
        printNode(node.index.get(), currentPrefix, true, "Index");
    }

    void visitExprStmt(ExprStmt& node) override {
        std::cout << "ExprStmt";
        printNode(node.expression.get(), currentPrefix, true);
    }

    void visitVarDeclStmt(VarDeclStmt& node) override {
        std::cout << "VarDeclStmt: " << node.name.lexeme;
        if (node.initializer) {
            printNode(node.initializer.get(), currentPrefix, true, "Init");
        }
    }

    void visitBlockStmt(BlockStmt& node) override {
        std::cout << "BlockStmt";
        for (size_t i = 0; i < node.statements.size(); ++i) {
            printNode(node.statements[i].get(), currentPrefix, i == node.statements.size() - 1);
        }
    }

    void visitIfStmt(IfStmt& node) override {
        std::cout << "IfStmt";
        printNode(node.condition.get(), currentPrefix, false, "Cond");
        printNode(node.thenBranch.get(), currentPrefix, !node.elseBranch, "Then");
        if (node.elseBranch) {
            printNode(node.elseBranch.get(), currentPrefix, true, "Else");
        }
    }

    void visitDuringStmt(DuringStmt& node) override {
        std::cout << "DuringStmt";
        printNode(node.condition.get(), currentPrefix, false, "Cond");
        printNode(node.body.get(), currentPrefix, true, "Body");
    }

    void visitForStmt(ForStmt& node) override {
        std::cout << "ForStmt: " << node.varName.lexeme;
        printNode(node.startExpr.get(), currentPrefix, false, "From");
        printNode(node.endExpr.get(), currentPrefix, false, "To");
        printNode(node.body.get(), currentPrefix, true, "Body");
    }

    void visitFunctionStmt(FunctionStmt& node) override {
        std::cout << "FunctionStmt: " << node.name.lexeme;
        printNode(node.body.get(), currentPrefix, true, "Body");
    }

    void visitReturnStmt(ReturnStmt& node) override {
        std::cout << "ReturnStmt";
        if (node.value) {
            printNode(node.value.get(), currentPrefix, true, "Value");
        }
    }

    void visitDisplayStmt(DisplayStmt& node) override {
        std::cout << "DisplayStmt";
        printNode(node.expression.get(), currentPrefix, true);
    }

    void visitDisplayLnStmt(DisplayLnStmt& node) override {
        std::cout << "DisplayLnStmt";
        printNode(node.expression.get(), currentPrefix, true);
    }
};

} // namespace AST
