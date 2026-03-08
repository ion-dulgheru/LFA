import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Lexer {

    private static final Map<String, TokenType> KEYWORDS = new HashMap<>();

    static {
        KEYWORDS.put("let",    TokenType.LET);
        KEYWORDS.put("fn",     TokenType.FN);
        KEYWORDS.put("return", TokenType.RETURN);
        KEYWORDS.put("if",     TokenType.IF);
        KEYWORDS.put("else",   TokenType.ELSE);
        KEYWORDS.put("while",  TokenType.WHILE);
        KEYWORDS.put("for",    TokenType.FOR);
        KEYWORDS.put("print",  TokenType.PRINT);
        KEYWORDS.put("true",   TokenType.BOOLEAN);
        KEYWORDS.put("false",  TokenType.BOOLEAN);
        KEYWORDS.put("sin",    TokenType.SIN);
        KEYWORDS.put("cos",    TokenType.COS);
        KEYWORDS.put("tan",    TokenType.TAN);
        KEYWORDS.put("sqrt",   TokenType.SQRT);
        KEYWORDS.put("abs",    TokenType.ABS);
        KEYWORDS.put("log",    TokenType.LOG);
        KEYWORDS.put("pow",    TokenType.POW);
    }

    private final String source;
    private int position;
    private int line;
    private int column;
    private char current;

    public Lexer(String source) {
        this.source   = source;
        this.position = 0;
        this.line     = 1;
        this.column   = 1;
        this.current  = source.isEmpty() ? '\0' : source.charAt(0);
    }

    public List<Token> tokenize() {
        List<Token> tokens = new ArrayList<>();
        Token token;
        do {
            token = nextToken();
            tokens.add(token);
        } while (token.type != TokenType.EOF);
        return tokens;
    }

    private Token nextToken() {
        skipWhitespaceAndNewlines();

        if (current == '\0') return makeToken(TokenType.EOF, "EOF");

        if (current == '/' && peek() == '/') return readSingleLineComment();
        if (current == '/' && peek() == '*') return readMultiLineComment();
        if (current == '"') return readString();
        if (Character.isDigit(current)) return readNumber();
        if (Character.isLetter(current) || current == '_') return readIdentifier();

        return readOperatorOrDelimiter();
    }

    private Token readNumber() {
        int startCol = column;
        StringBuilder sb = new StringBuilder();
        boolean isFloat = false;

        while (current != '\0' && (Character.isDigit(current) || current == '.')) {
            if (current == '.') {
                if (isFloat) break;
                isFloat = true;
            }
            sb.append(current);
            advance();
        }

        return new Token(isFloat ? TokenType.FLOAT : TokenType.INTEGER, sb.toString(), line, startCol);
    }

    private Token readIdentifier() {
        int startCol = column;
        StringBuilder sb = new StringBuilder();

        while (current != '\0' && (Character.isLetterOrDigit(current) || current == '_')) {
            sb.append(current);
            advance();
        }

        String word = sb.toString();
        TokenType type = KEYWORDS.getOrDefault(word, TokenType.IDENTIFIER);
        return new Token(type, word, line, startCol);
    }

    private Token readString() {
        int startCol = column;
        StringBuilder sb = new StringBuilder();
        advance();

        while (current != '\0' && current != '"') {
            if (current == '\\') {
                advance();
                switch (current) {
                    case 'n':  sb.append('\n'); break;
                    case 't':  sb.append('\t'); break;
                    case '\\': sb.append('\\'); break;
                    case '"':  sb.append('"');  break;
                    default:
                        sb.append('\\');
                        sb.append(current);
                }
            } else {
                sb.append(current);
            }
            advance();
        }

        if (current == '"') advance();
        else return new Token(TokenType.ILLEGAL, "Unterminated string", line, startCol);

        return new Token(TokenType.STRING, sb.toString(), line, startCol);
    }

    private Token readSingleLineComment() {
        int startCol = column;
        StringBuilder sb = new StringBuilder();
        advance(); advance();

        while (current != '\0' && current != '\n') {
            sb.append(current);
            advance();
        }

        return new Token(TokenType.COMMENT, sb.toString().trim(), line, startCol);
    }

    private Token readMultiLineComment() {
        int startCol = column;
        StringBuilder sb = new StringBuilder();
        advance(); advance();

        while (current != '\0') {
            if (current == '*' && peek() == '/') {
                advance(); advance();
                break;
            }
            sb.append(current);
            advance();
        }

        return new Token(TokenType.COMMENT, sb.toString().trim(), line, startCol);
    }

    private Token readOperatorOrDelimiter() {
        int startCol = column;
        char ch = current;
        advance();

        switch (ch) {
            case '+':
                if (current == '+') { advance(); return new Token(TokenType.INCREMENT,    "++", line, startCol); }
                if (current == '=') { advance(); return new Token(TokenType.PLUS_ASSIGN,  "+=", line, startCol); }
                return new Token(TokenType.PLUS,     "+",  line, startCol);

            case '-':
                if (current == '-') { advance(); return new Token(TokenType.DECREMENT,    "--", line, startCol); }
                if (current == '=') { advance(); return new Token(TokenType.MINUS_ASSIGN, "-=", line, startCol); }
                return new Token(TokenType.MINUS,    "-",  line, startCol);

            case '*':
                if (current == '*') { advance(); return new Token(TokenType.POWER,        "**", line, startCol); }
                if (current == '=') { advance(); return new Token(TokenType.MUL_ASSIGN,   "*=", line, startCol); }
                return new Token(TokenType.MULTIPLY, "*",  line, startCol);

            case '/':
                if (current == '=') { advance(); return new Token(TokenType.DIV_ASSIGN,   "/=", line, startCol); }
                return new Token(TokenType.DIVIDE,   "/",  line, startCol);

            case '%': return new Token(TokenType.MODULO,    "%",  line, startCol);

            case '=':
                if (current == '=') { advance(); return new Token(TokenType.EQ,     "==", line, startCol); }
                return new Token(TokenType.ASSIGN,   "=",  line, startCol);

            case '!':
                if (current == '=') { advance(); return new Token(TokenType.NOT_EQ, "!=", line, startCol); }
                return new Token(TokenType.NOT,      "!",  line, startCol);

            case '<':
                if (current == '=') { advance(); return new Token(TokenType.LT_EQ,  "<=", line, startCol); }
                return new Token(TokenType.LT,       "<",  line, startCol);

            case '>':
                if (current == '=') { advance(); return new Token(TokenType.GT_EQ,  ">=", line, startCol); }
                return new Token(TokenType.GT,       ">",  line, startCol);

            case '&':
                if (current == '&') { advance(); return new Token(TokenType.AND, "&&", line, startCol); }
                break;
            case '|':
                if (current == '|') { advance(); return new Token(TokenType.OR,  "||", line, startCol); }
                break;

            case ';': return new Token(TokenType.SEMICOLON, ";", line, startCol);
            case ',': return new Token(TokenType.COMMA,     ",", line, startCol);
            case '.': return new Token(TokenType.DOT,       ".", line, startCol);
            case ':': return new Token(TokenType.COLON,     ":", line, startCol);
            case '(': return new Token(TokenType.LPAREN,    "(", line, startCol);
            case ')': return new Token(TokenType.RPAREN,    ")", line, startCol);
            case '{': return new Token(TokenType.LBRACE,    "{", line, startCol);
            case '}': return new Token(TokenType.RBRACE,    "}", line, startCol);
            case '[': return new Token(TokenType.LBRACKET,  "[", line, startCol);
            case ']': return new Token(TokenType.RBRACKET,  "]", line, startCol);
        }

        return new Token(TokenType.ILLEGAL, String.valueOf(ch), line, startCol);
    }

    private void advance() {
        if (current == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        position++;
        current = (position < source.length()) ? source.charAt(position) : '\0';
    }

    private char peek() {
        int next = position + 1;
        return (next < source.length()) ? source.charAt(next) : '\0';
    }

    private void skipWhitespaceAndNewlines() {
        while (current != '\0' && Character.isWhitespace(current)) {
            advance();
        }
    }

    private Token makeToken(TokenType type, String value) {
        return new Token(type, value, line, column);
    }
}