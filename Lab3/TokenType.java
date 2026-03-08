public enum TokenType {

    INTEGER,
    FLOAT,
    STRING,
    BOOLEAN,
    IDENTIFIER,

    LET,
    FN,
    RETURN,
    IF,
    ELSE,
    WHILE,
    FOR,
    PRINT,

    SIN,
    COS,
    TAN,
    SQRT,
    ABS,
    LOG,
    POW,

    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    MODULO,
    POWER,

    ASSIGN,
    PLUS_ASSIGN,
    MINUS_ASSIGN,
    MUL_ASSIGN,
    DIV_ASSIGN,

    INCREMENT,
    DECREMENT,

    EQ,
    NOT_EQ,
    LT,
    GT,
    LT_EQ,
    GT_EQ,

    AND,
    OR,
    NOT,

    SEMICOLON,
    COMMA,
    DOT,
    COLON,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,

    COMMENT,
    NEWLINE,
    EOF,
    ILLEGAL
}