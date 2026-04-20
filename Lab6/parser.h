#pragma once
#include "lexer.h"
#include "ast.h"
#include <stdexcept>
#include <set>

// ══════════════════════════════════════════════════════════════════
//  RECURSIVE DESCENT PARSER
//
//  Grammar:
//  program      → statement* EOF
//  statement    → fnDef | letDecl | compoundAssign | assignment
//               | returnStmt | printStmt | ifStmt | whileStmt | forStmt
//  fnDef        → 'fn' IDENTIFIER '(' params ')' block
//  params       → ( IDENTIFIER (',' IDENTIFIER)* )?
//  letDecl      → 'let' IDENTIFIER '=' expression ';'
//  assignment   → IDENTIFIER '=' expression ';'
//  compoundAssign → IDENTIFIER ('+=' | '-=' | '*=' | '/=') expression ';'
//  returnStmt   → 'return' expression ';'
//  printStmt    → 'print' '(' expression ')' ';'
//  ifStmt       → 'if' '(' expression ')' block ('else' block)?
//  whileStmt    → 'while' '(' expression ')' block
//  forStmt      → 'for' '(' forInit expression ';' forUpdate ')' block
//  forInit      → letDecl | assignment
//  forUpdate    → compoundAssignExpr | IDENTIFIER ('++'|'--')
//  block        → '{' statement* '}'
//
//  expression   → or
//  or           → and ( '||' and )*
//  and          → equality ( '&&' equality )*
//  equality     → comparison ( ('==' | '!=') comparison )*
//  comparison   → addition ( ('<'|'>'|'<='|'>=') addition )*
//  addition     → multiply ( ('+'|'-') multiply )*
//  multiply     → unary ( ('*'|'/'|'%'|'**') unary )*
//  unary        → ('-'|'!'|'++'|'--') unary | postfix
//  postfix      → primary ('++' | '--')?
//  primary      → INTEGER | FLOAT | STRING | BOOLEAN | IDENTIFIER
//               | mathCall | '(' expression ')'
//  mathCall     → ('sin'|'cos'|'tan'|'sqrt'|'abs'|'log'|'pow')
//                  '(' expression (',' expression)* ')'
// ══════════════════════════════════════════════════════════════════

class Parser {
public:
    explicit Parser(std::vector<Token> toks) : tokens(std::move(toks)), pos(0) {}

    std::unique_ptr<Program> parse() {
        auto prog = std::make_unique<Program>();
        // Skip leading comments
        while (check(TokenType::COMMENT)) ++pos;
        while (!check(TokenType::EOF_TOKEN)) {
            while (check(TokenType::COMMENT)) ++pos;
            if (check(TokenType::EOF_TOKEN)) break;
            prog->statements.push_back(parseStatement());
        }
        return prog;
    }

private:
    std::vector<Token> tokens;
    int pos;

    const Token& cur() const { return tokens[pos]; }
    bool check(TokenType t)  const { return cur().type == t; }

    bool match(TokenType t) {
        if (check(t)) { ++pos; return true; }
        return false;
    }

    Token expect(TokenType t, const std::string& msg) {
        if (!check(t))
            throw std::runtime_error(
                "Parse error at line " + std::to_string(cur().line) +
                ": expected " + msg + ", got \"" + cur().value + "\"");
        return tokens[pos++];
    }

    // ── Helpers ───────────────────────────────────────────────────
    bool isMathBuiltin() const {
        return check(TokenType::SIN)  || check(TokenType::COS) ||
               check(TokenType::TAN)  || check(TokenType::SQRT)||
               check(TokenType::ABS)  || check(TokenType::LOG) ||
               check(TokenType::POW);
    }

    bool isCompoundAssign() const {
        return check(TokenType::PLUS_ASSIGN)  || check(TokenType::MINUS_ASSIGN) ||
               check(TokenType::MUL_ASSIGN)   || check(TokenType::DIV_ASSIGN);
    }

    // ── Statements ────────────────────────────────────────────────
    NodePtr parseStatement() {
        if (check(TokenType::FN))     return parseFnDef();
        if (check(TokenType::LET))    return parseLetDecl();
        if (check(TokenType::RETURN)) return parseReturn();
        if (check(TokenType::PRINT))  return parsePrint();
        if (check(TokenType::IF))     return parseIf();
        if (check(TokenType::WHILE))  return parseWhile();
        if (check(TokenType::FOR))    return parseFor();

        // assignment or compound assignment
        if (check(TokenType::IDENTIFIER)) {
            if (pos+1 < (int)tokens.size()) {
                TokenType next = tokens[pos+1].type;
                if (next == TokenType::ASSIGN)        return parseAssignment();
                if (next == TokenType::PLUS_ASSIGN  ||
                    next == TokenType::MINUS_ASSIGN  ||
                    next == TokenType::MUL_ASSIGN    ||
                    next == TokenType::DIV_ASSIGN)    return parseCompoundAssign();
                // i++ ; or i-- ;
                if (next == TokenType::INCREMENT || next == TokenType::DECREMENT) {
                    std::string name = tokens[pos++].value;
                    std::string op   = tokens[pos++].value;
                    expect(TokenType::SEMICOLON, "';'");
                    return std::make_unique<PostfixOp>(
                        op, std::make_unique<Variable>(name));
                }
            }
        }
        throw std::runtime_error(
            "Parse error at line " + std::to_string(cur().line) +
            ": unexpected token \"" + cur().value + "\"");
    }

    // fn name(params) { body }
    NodePtr parseFnDef() {
        expect(TokenType::FN, "'fn'");
        auto node = std::make_unique<FunctionDef>();
        node->name = expect(TokenType::IDENTIFIER, "function name").value;
        expect(TokenType::LPAREN, "'('");
        if (!check(TokenType::RPAREN)) {
            node->params.push_back(expect(TokenType::IDENTIFIER, "param").value);
            while (match(TokenType::COMMA))
                node->params.push_back(expect(TokenType::IDENTIFIER, "param").value);
        }
        expect(TokenType::RPAREN, "')'");
        node->body = parseBlock();
        return node;
    }

    // let x = expr ;
    NodePtr parseLetDecl() {
        expect(TokenType::LET, "'let'");
        std::string name = expect(TokenType::IDENTIFIER, "identifier").value;
        expect(TokenType::ASSIGN, "'='");
        auto val = parseExpression();
        expect(TokenType::SEMICOLON, "';'");
        return std::make_unique<LetDecl>(name, std::move(val));
    }

    // x = expr ;
    NodePtr parseAssignment() {
        std::string name = tokens[pos++].value;
        expect(TokenType::ASSIGN, "'='");
        auto val = parseExpression();
        expect(TokenType::SEMICOLON, "';'");
        return std::make_unique<Assignment>(name, std::move(val));
    }

    // x += expr ;
    NodePtr parseCompoundAssign() {
        std::string name = tokens[pos++].value;
        std::string op   = tokens[pos++].value;
        auto val = parseExpression();
        expect(TokenType::SEMICOLON, "';'");
        return std::make_unique<CompoundAssign>(name, op, std::move(val));
    }

    // return expr ;
    NodePtr parseReturn() {
        expect(TokenType::RETURN, "'return'");
        auto val = parseExpression();
        expect(TokenType::SEMICOLON, "';'");
        return std::make_unique<ReturnStmt>(std::move(val));
    }

    // print(expr) ;
    NodePtr parsePrint() {
        expect(TokenType::PRINT,    "'print'");
        expect(TokenType::LPAREN,   "'('");
        auto expr = parseExpression();
        expect(TokenType::RPAREN,   "')'");
        expect(TokenType::SEMICOLON,"';'");
        return std::make_unique<PrintStmt>(std::move(expr));
    }

    // if (cond) block [else block]
    NodePtr parseIf() {
        expect(TokenType::IF,    "'if'");
        expect(TokenType::LPAREN,"'('");
        auto cond = parseExpression();
        expect(TokenType::RPAREN,"')'");
        auto node = std::make_unique<IfStmt>();
        node->condition  = std::move(cond);
        node->thenBranch = parseBlock();
        if (match(TokenType::ELSE)) node->elseBranch = parseBlock();
        return node;
    }

    // while (cond) block
    NodePtr parseWhile() {
        expect(TokenType::WHILE, "'while'");
        expect(TokenType::LPAREN,"'('");
        auto cond = parseExpression();
        expect(TokenType::RPAREN,"')'");
        auto node = std::make_unique<WhileStmt>();
        node->condition = std::move(cond);
        node->body      = parseBlock();
        return node;
    }

    // for (let i = 0; i < n; i++) block
    NodePtr parseFor() {
        expect(TokenType::FOR,   "'for'");
        expect(TokenType::LPAREN,"'('");
        auto node = std::make_unique<ForStmt>();

        // init: let x = expr ;  OR  x = expr ;
        if (check(TokenType::LET))        node->init = parseLetDecl();
        else                              node->init = parseAssignment();

        // condition
        node->condition = parseExpression();
        expect(TokenType::SEMICOLON,"';'");

        // update: x++ / x-- / x += expr
        std::string uname = expect(TokenType::IDENTIFIER,"identifier").value;
        if (check(TokenType::INCREMENT) || check(TokenType::DECREMENT)) {
            std::string op = tokens[pos++].value;
            node->update = std::make_unique<PostfixOp>(
                op, std::make_unique<Variable>(uname));
        } else if (isCompoundAssign()) {
            std::string op = tokens[pos++].value;
            auto val = parseExpression();
            node->update = std::make_unique<CompoundAssign>(uname, op, std::move(val));
        } else {
            throw std::runtime_error("Expected for-update expression");
        }

        expect(TokenType::RPAREN,"')'");
        node->body = parseBlock();
        return node;
    }

    std::vector<NodePtr> parseBlock() {
        expect(TokenType::LBRACE,"'{'");
        std::vector<NodePtr> stmts;
        while (check(TokenType::COMMENT)) ++pos;
        while (!check(TokenType::RBRACE) && !check(TokenType::EOF_TOKEN)) {
            while (check(TokenType::COMMENT)) ++pos;
            if (check(TokenType::RBRACE)) break;
            stmts.push_back(parseStatement());
        }
        expect(TokenType::RBRACE,"'}'");
        return stmts;
    }

    // ── Expressions ───────────────────────────────────────────────
    NodePtr parseExpression() { return parseOr(); }

    NodePtr parseOr() {
        auto left = parseAnd();
        while (check(TokenType::OR)) {
            std::string op = tokens[pos++].value;
            left = std::make_unique<BinaryOp>(op, std::move(left), parseAnd());
        }
        return left;
    }

    NodePtr parseAnd() {
        auto left = parseEquality();
        while (check(TokenType::AND)) {
            std::string op = tokens[pos++].value;
            left = std::make_unique<BinaryOp>(op, std::move(left), parseEquality());
        }
        return left;
    }

    NodePtr parseEquality() {
        auto left = parseComparison();
        while (check(TokenType::EQ) || check(TokenType::NOT_EQ)) {
            std::string op = tokens[pos++].value;
            left = std::make_unique<BinaryOp>(op, std::move(left), parseComparison());
        }
        return left;
    }

    NodePtr parseComparison() {
        auto left = parseAddition();
        while (check(TokenType::LT) || check(TokenType::GT) ||
               check(TokenType::LT_EQ) || check(TokenType::GT_EQ)) {
            std::string op = tokens[pos++].value;
            left = std::make_unique<BinaryOp>(op, std::move(left), parseAddition());
        }
        return left;
    }

    NodePtr parseAddition() {
        auto left = parseMultiply();
        while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
            std::string op = tokens[pos++].value;
            left = std::make_unique<BinaryOp>(op, std::move(left), parseMultiply());
        }
        return left;
    }

    NodePtr parseMultiply() {
        auto left = parseUnary();
        while (check(TokenType::MULTIPLY) || check(TokenType::DIVIDE) ||
               check(TokenType::MODULO)   || check(TokenType::POWER)) {
            std::string op = tokens[pos++].value;
            left = std::make_unique<BinaryOp>(op, std::move(left), parseUnary());
        }
        return left;
    }

    NodePtr parseUnary() {
        if (check(TokenType::MINUS) || check(TokenType::NOT) ||
            check(TokenType::INCREMENT) || check(TokenType::DECREMENT)) {
            std::string op = tokens[pos++].value;
            return std::make_unique<UnaryOp>(op, parseUnary());
        }
        return parsePostfix();
    }

    NodePtr parsePostfix() {
        auto node = parsePrimary();
        if (check(TokenType::INCREMENT) || check(TokenType::DECREMENT)) {
            std::string op = tokens[pos++].value;
            return std::make_unique<PostfixOp>(op, std::move(node));
        }
        return node;
    }

    NodePtr parsePrimary() {
        if (check(TokenType::INTEGER)) {
            int v = std::stoi(tokens[pos].value); ++pos;
            return std::make_unique<IntLiteral>(v);
        }
        if (check(TokenType::FLOAT)) {
            double v = std::stod(tokens[pos].value); ++pos;
            return std::make_unique<FloatLiteral>(v);
        }
        if (check(TokenType::STRING)) {
            std::string v = tokens[pos++].value;
            return std::make_unique<StringLiteral>(v);
        }
        if (check(TokenType::BOOLEAN)) {
            bool v = (tokens[pos++].value == "true");
            return std::make_unique<BoolLiteral>(v);
        }
        if (isMathBuiltin()) {
            std::string fname = tokens[pos++].value;
            expect(TokenType::LPAREN,"'('");
            auto call = std::make_unique<MathCall>(fname);
            call->args.push_back(parseExpression());
            while (match(TokenType::COMMA))
                call->args.push_back(parseExpression());
            expect(TokenType::RPAREN,"')'");
            return call;
        }
        if (check(TokenType::IDENTIFIER)) {
            std::string name = tokens[pos++].value;
            // function call: name(args)
            if (check(TokenType::LPAREN)) {
                ++pos;
                auto call = std::make_unique<FunctionCall>(name);
                if (!check(TokenType::RPAREN)) {
                    call->args.push_back(parseExpression());
                    while (match(TokenType::COMMA))
                        call->args.push_back(parseExpression());
                }
                expect(TokenType::RPAREN, "')'");
                return call;
            }
            return std::make_unique<Variable>(name);
        }
        if (match(TokenType::LPAREN)) {
            auto expr = parseExpression();
            expect(TokenType::RPAREN,"')'");
            return expr;
        }
        throw std::runtime_error(
            "Parse error at line " + std::to_string(cur().line) +
            ": unexpected \"" + cur().value + "\"");
    }
};