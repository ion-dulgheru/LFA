#pragma once
#include <string>
#include <vector>
#include <regex>
#include <stdexcept>

// ══════════════════════════════════════════════════════════════════
//  TOKEN TYPE  — mirrors the Java Lab 3 TokenType enum exactly
// ══════════════════════════════════════════════════════════════════
enum class TokenType {
    // Literals
    INTEGER, FLOAT, STRING, BOOLEAN,
    // Keywords
    LET, FN, RETURN,
    IF, ELSE, WHILE, FOR, PRINT,
    // Math built-ins
    SIN, COS, TAN, SQRT, ABS, LOG, POW,
    // Identifier
    IDENTIFIER,
    // Arithmetic
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO, POWER,
    // Compound assignment
    PLUS_ASSIGN, MINUS_ASSIGN, MUL_ASSIGN, DIV_ASSIGN,
    // Increment / Decrement
    INCREMENT, DECREMENT,
    // Comparison
    EQ, NOT_EQ, LT, GT, LT_EQ, GT_EQ,
    // Logical
    AND, OR, NOT,
    // Assignment
    ASSIGN,
    // Delimiters
    SEMICOLON, COMMA, DOT, COLON,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    // Other
    COMMENT, EOF_TOKEN, ILLEGAL
};

inline std::string tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::INTEGER:       return "INTEGER";
        case TokenType::FLOAT:         return "FLOAT";
        case TokenType::STRING:        return "STRING";
        case TokenType::BOOLEAN:       return "BOOLEAN";
        case TokenType::LET:           return "LET";
        case TokenType::FN:            return "FN";
        case TokenType::RETURN:        return "RETURN";
        case TokenType::IF:            return "IF";
        case TokenType::ELSE:          return "ELSE";
        case TokenType::WHILE:         return "WHILE";
        case TokenType::FOR:           return "FOR";
        case TokenType::PRINT:         return "PRINT";
        case TokenType::SIN:           return "SIN";
        case TokenType::COS:           return "COS";
        case TokenType::TAN:           return "TAN";
        case TokenType::SQRT:          return "SQRT";
        case TokenType::ABS:           return "ABS";
        case TokenType::LOG:           return "LOG";
        case TokenType::POW:           return "POW";
        case TokenType::IDENTIFIER:    return "IDENTIFIER";
        case TokenType::PLUS:          return "PLUS";
        case TokenType::MINUS:         return "MINUS";
        case TokenType::MULTIPLY:      return "MULTIPLY";
        case TokenType::DIVIDE:        return "DIVIDE";
        case TokenType::MODULO:        return "MODULO";
        case TokenType::POWER:         return "POWER";
        case TokenType::PLUS_ASSIGN:   return "PLUS_ASSIGN";
        case TokenType::MINUS_ASSIGN:  return "MINUS_ASSIGN";
        case TokenType::MUL_ASSIGN:    return "MUL_ASSIGN";
        case TokenType::DIV_ASSIGN:    return "DIV_ASSIGN";
        case TokenType::INCREMENT:     return "INCREMENT";
        case TokenType::DECREMENT:     return "DECREMENT";
        case TokenType::EQ:            return "EQ";
        case TokenType::NOT_EQ:        return "NOT_EQ";
        case TokenType::LT:            return "LT";
        case TokenType::GT:            return "GT";
        case TokenType::LT_EQ:         return "LT_EQ";
        case TokenType::GT_EQ:         return "GT_EQ";
        case TokenType::AND:           return "AND";
        case TokenType::OR:            return "OR";
        case TokenType::NOT:           return "NOT";
        case TokenType::ASSIGN:        return "ASSIGN";
        case TokenType::SEMICOLON:     return "SEMICOLON";
        case TokenType::COMMA:         return "COMMA";
        case TokenType::DOT:           return "DOT";
        case TokenType::COLON:         return "COLON";
        case TokenType::LPAREN:        return "LPAREN";
        case TokenType::RPAREN:        return "RPAREN";
        case TokenType::LBRACE:        return "LBRACE";
        case TokenType::RBRACE:        return "RBRACE";
        case TokenType::LBRACKET:      return "LBRACKET";
        case TokenType::RBRACKET:      return "RBRACKET";
        case TokenType::COMMENT:       return "COMMENT";
        case TokenType::EOF_TOKEN:     return "EOF";
        default:                       return "ILLEGAL";
    }
}

// ══════════════════════════════════════════════════════════════════
//  TOKEN
// ══════════════════════════════════════════════════════════════════
struct Token {
    TokenType   type;
    std::string value;
    int         line;
    int         column;

    Token(TokenType t, std::string v, int l, int c = 0)
        : type(t), value(std::move(v)), line(l), column(c) {}

    std::string toString() const {
        return "Token(" + tokenTypeName(type) +
               ", \"" + value +
               "\", line=" + std::to_string(line) +
               ", col="  + std::to_string(column) + ")";
    }
};

// ══════════════════════════════════════════════════════════════════
//  LEXER  — regex-based, same token set as Java Lab 3
// ══════════════════════════════════════════════════════════════════
class Lexer {
public:
    struct Rule {
        std::regex pattern;
        TokenType  type;
        Rule(const std::string& pat, TokenType t)
            : pattern("^(?:" + pat + ")"), type(t) {}
    };

    explicit Lexer(const std::string& source) : src(source), pos(0), line(1), col(1) {
        rules = {
            // Whitespace — skip silently (ILLEGAL used as sentinel)
            { "[ \\t\\r]+",                   TokenType::ILLEGAL      },
            // Comments — kept as COMMENT tokens (same as Java)
            { "//[^\\n]*",                    TokenType::COMMENT      },
            { "/\\*[\\s\\S]*?\\*/",           TokenType::COMMENT      },
            // Literals
            { "[0-9]+\\.[0-9]+",              TokenType::FLOAT        },
            { "[0-9]+",                       TokenType::INTEGER      },
            { "\"(?:[^\"\\\\]|\\\\.)*\"",     TokenType::STRING       },
            // Booleans before identifier
            { "true\\b",                      TokenType::BOOLEAN      },
            { "false\\b",                     TokenType::BOOLEAN      },
            // Keywords before identifier
            { "let\\b",                       TokenType::LET          },
            { "fn\\b",                        TokenType::FN           },
            { "return\\b",                    TokenType::RETURN       },
            { "if\\b",                        TokenType::IF           },
            { "else\\b",                      TokenType::ELSE         },
            { "while\\b",                     TokenType::WHILE        },
            { "for\\b",                       TokenType::FOR          },
            { "print\\b",                     TokenType::PRINT        },
            // Math built-ins before identifier
            { "sin\\b",                       TokenType::SIN          },
            { "cos\\b",                       TokenType::COS          },
            { "tan\\b",                       TokenType::TAN          },
            { "sqrt\\b",                      TokenType::SQRT         },
            { "abs\\b",                       TokenType::ABS          },
            { "log\\b",                       TokenType::LOG          },
            { "pow\\b",                       TokenType::POW          },
            // Identifier
            { "[a-zA-Z_][a-zA-Z0-9_]*",      TokenType::IDENTIFIER   },
            // Multi-char operators (before single-char)
            { "\\+\\+",                       TokenType::INCREMENT    },
            { "--",                           TokenType::DECREMENT    },
            { "\\*\\*",                       TokenType::POWER        },
            { "\\+=",                         TokenType::PLUS_ASSIGN  },
            { "-=",                           TokenType::MINUS_ASSIGN },
            { "\\*=",                         TokenType::MUL_ASSIGN   },
            { "/=",                           TokenType::DIV_ASSIGN   },
            { "==",                           TokenType::EQ           },
            { "!=",                           TokenType::NOT_EQ       },
            { "<=",                           TokenType::LT_EQ        },
            { ">=",                           TokenType::GT_EQ        },
            { "&&",                           TokenType::AND          },
            { "\\|\\|",                       TokenType::OR           },
            // Single-char operators
            { "\\+",                          TokenType::PLUS         },
            { "-",                            TokenType::MINUS        },
            { "\\*",                          TokenType::MULTIPLY     },
            { "/",                            TokenType::DIVIDE       },
            { "%",                            TokenType::MODULO       },
            { "=",                            TokenType::ASSIGN       },
            { "<",                            TokenType::LT           },
            { ">",                            TokenType::GT           },
            { "!",                            TokenType::NOT          },
            // Delimiters
            { ";",                            TokenType::SEMICOLON    },
            { ",",                            TokenType::COMMA        },
            { "\\.",                          TokenType::DOT          },
            { ":",                            TokenType::COLON        },
            { "\\(",                          TokenType::LPAREN       },
            { "\\)",                          TokenType::RPAREN       },
            { "\\{",                          TokenType::LBRACE       },
            { "\\}",                          TokenType::RBRACE       },
            { "\\[",                          TokenType::LBRACKET     },
            { "\\]",                          TokenType::RBRACKET     },
        };
    }

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos < (int)src.size()) {
            if (src[pos] == '\n') { ++line; col = 1; ++pos; continue; }

            std::string rem = src.substr(pos);
            bool matched = false;

            for (const auto& rule : rules) {
                std::smatch m;
                if (std::regex_search(rem, m, rule.pattern)) {
                    std::string val = m[0].str();
                    if (rule.type != TokenType::ILLEGAL) // skip whitespace
                        tokens.emplace_back(rule.type, val, line, col);
                    col += (int)val.size();
                    pos += (int)val.size();
                    matched = true;
                    break;
                }
            }
            if (!matched)
                throw std::runtime_error(
                    "Unexpected character '" + std::string(1, src[pos]) +
                    "' at line " + std::to_string(line));
        }
        tokens.emplace_back(TokenType::EOF_TOKEN, "EOF", line, col);
        return tokens;
    }

private:
    std::string       src;
    int               pos, line, col;
    std::vector<Rule> rules;
};