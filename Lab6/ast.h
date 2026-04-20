#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
protected:
    static std::string pad(int n) { return std::string(n * 2, ' '); }
};
using NodePtr = std::unique_ptr<ASTNode>;

// ── Literals ──────────────────────────────────────────────────────
struct IntLiteral : ASTNode {
    int value;
    explicit IntLiteral(int v) : value(v) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "IntLiteral(" << value << ")\n";
    }
};

struct FloatLiteral : ASTNode {
    double value;
    explicit FloatLiteral(double v) : value(v) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "FloatLiteral(" << value << ")\n";
    }
};

struct StringLiteral : ASTNode {
    std::string value;
    explicit StringLiteral(std::string v) : value(std::move(v)) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "StringLiteral(" << value << ")\n";
    }
};

struct BoolLiteral : ASTNode {
    bool value;
    explicit BoolLiteral(bool v) : value(v) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "BoolLiteral(" << (value?"true":"false") << ")\n";
    }
};

// ── Expressions ───────────────────────────────────────────────────

// Variable reference: x
struct Variable : ASTNode {
    std::string name;
    explicit Variable(std::string n) : name(std::move(n)) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "Variable(" << name << ")\n";
    }
};

// Binary operation: left OP right
struct BinaryOp : ASTNode {
    std::string op;
    NodePtr left, right;
    BinaryOp(std::string o, NodePtr l, NodePtr r)
        : op(std::move(o)), left(std::move(l)), right(std::move(r)) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "BinaryOp(" << op << ")\n";
        left ->print(i+1);
        right->print(i+1);
    }
};

// Unary operation: OP expr  (also prefix ++ / --)
struct UnaryOp : ASTNode {
    std::string op;
    NodePtr operand;
    UnaryOp(std::string o, NodePtr e) : op(std::move(o)), operand(std::move(e)) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "UnaryOp(" << op << ")\n";
        operand->print(i+1);
    }
};

// Postfix ++ / --: expr OP
struct PostfixOp : ASTNode {
    std::string op;
    NodePtr operand;
    PostfixOp(std::string o, NodePtr e) : op(std::move(o)), operand(std::move(e)) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "PostfixOp(" << op << ")\n";
        operand->print(i+1);
    }
};

// Math built-in call: sin(x), pow(x, y), etc.
struct MathCall : ASTNode {
    std::string            func;   // sin / cos / tan / sqrt / abs / log / pow
    std::vector<NodePtr>   args;
    explicit MathCall(std::string f) : func(std::move(f)) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "MathCall(" << func << ")\n";
        for (const auto& a : args) a->print(i+1);
    }
};

// ── Statements ────────────────────────────────────────────────────

// let x = expr ;
struct LetDecl : ASTNode {
    std::string name;
    NodePtr     value;
    LetDecl(std::string n, NodePtr v) : name(std::move(n)), value(std::move(v)) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "LetDecl(" << name << ")\n";
        value->print(i+1);
    }
};

// x = expr ;
struct Assignment : ASTNode {
    std::string name;
    NodePtr     value;
    Assignment(std::string n, NodePtr v) : name(std::move(n)), value(std::move(v)) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "Assignment(" << name << ")\n";
        value->print(i+1);
    }
};

// x += expr ;   x -= expr ;   x *= expr ;   x /= expr ;
struct CompoundAssign : ASTNode {
    std::string name;
    std::string op;
    NodePtr     value;
    CompoundAssign(std::string n, std::string o, NodePtr v)
        : name(std::move(n)), op(std::move(o)), value(std::move(v)) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "CompoundAssign(" << name << " " << op << ")\n";
        value->print(i+1);
    }
};

// print(expr) ;
struct PrintStmt : ASTNode {
    NodePtr expr;
    explicit PrintStmt(NodePtr e) : expr(std::move(e)) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "PrintStmt\n";
        expr->print(i+1);
    }
};

// return expr ;
struct ReturnStmt : ASTNode {
    NodePtr expr;
    explicit ReturnStmt(NodePtr e) : expr(std::move(e)) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "ReturnStmt\n";
        expr->print(i+1);
    }
};

// if (cond) { then } [ else { els } ]
struct IfStmt : ASTNode {
    NodePtr              condition;
    std::vector<NodePtr> thenBranch;
    std::vector<NodePtr> elseBranch;
    void print(int i=0) const override {
        std::cout << pad(i) << "IfStmt\n";
        std::cout << pad(i+1) << "[condition]\n";
        condition->print(i+2);
        std::cout << pad(i+1) << "[then]\n";
        for (const auto& s : thenBranch) s->print(i+2);
        if (!elseBranch.empty()) {
            std::cout << pad(i+1) << "[else]\n";
            for (const auto& s : elseBranch) s->print(i+2);
        }
    }
};

// while (cond) { body }
struct WhileStmt : ASTNode {
    NodePtr              condition;
    std::vector<NodePtr> body;
    void print(int i=0) const override {
        std::cout << pad(i) << "WhileStmt\n";
        std::cout << pad(i+1) << "[condition]\n";
        condition->print(i+2);
        std::cout << pad(i+1) << "[body]\n";
        for (const auto& s : body) s->print(i+2);
    }
};

// for (init; cond; update) { body }
struct ForStmt : ASTNode {
    NodePtr              init;
    NodePtr              condition;
    NodePtr              update;
    std::vector<NodePtr> body;
    void print(int i=0) const override {
        std::cout << pad(i) << "ForStmt\n";
        std::cout << pad(i+1) << "[init]\n";    init->print(i+2);
        std::cout << pad(i+1) << "[condition]\n"; condition->print(i+2);
        std::cout << pad(i+1) << "[update]\n";  update->print(i+2);
        std::cout << pad(i+1) << "[body]\n";
        for (const auto& s : body) s->print(i+2);
    }
};

// fn name(params) { body }
struct FunctionDef : ASTNode {
    std::string              name;
    std::vector<std::string> params;
    std::vector<NodePtr>     body;
    void print(int i=0) const override {
        std::cout << pad(i) << "FunctionDef(" << name << ")\n";
        std::cout << pad(i+1) << "[params]";
        for (const auto& p : params) std::cout << " " << p;
        std::cout << "\n";
        std::cout << pad(i+1) << "[body]\n";
        for (const auto& s : body) s->print(i+2);
    }
};

// Program root
struct Program : ASTNode {
    std::vector<NodePtr> statements;
    void print(int i=0) const override {
        std::cout << pad(i) << "Program\n";
        for (const auto& s : statements) s->print(i+1);
    }
};

// User-defined function call: name(args)
struct FunctionCall : ASTNode {
    std::string          name;
    std::vector<NodePtr> args;
    explicit FunctionCall(std::string n) : name(std::move(n)) {}
    void print(int i=0) const override {
        std::cout << pad(i) << "FunctionCall(" << name << ")\n";
        for (const auto& a : args) a->print(i+1);
    }
};