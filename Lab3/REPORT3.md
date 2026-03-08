# Laboratory Work 3: Lexer / Scanner

**Course:** Formal Languages & Finite Automata  
**Author:** Dulgheru Ion  
**Group:** FAF-241  

## Theory

A lexer, also known as a scanner or tokenizer, is the first stage of a compiler or interpreter. Its responsibility is to read the raw source code as a linear stream of characters and group those characters into meaningful units called tokens. Each token represents a single logical element of the language, such as a keyword, an identifier, a number, an operator, or a delimiter. The process of producing these tokens from raw text is called lexical analysis, and it forms the essential first step that makes all further processing of source code possible.

Before the invention of lexers, early compilers had to parse raw character streams directly, which made the grammar rules enormously complex and error-prone. By introducing a dedicated lexical analysis phase, compiler designers were able to cleanly separate two distinct concerns: recognizing the vocabulary of the language on one side, and understanding its grammatical structure on the other. This separation of concerns is one of the foundational principles of modern compiler design.

The distinction between a lexeme and a token is important. A lexeme is the raw sequence of characters extracted from the source, for example the text `let`. A token is the structured representation of that lexeme, pairing it with a category and optional metadata such as its position in the file. So the lexeme `let` becomes `Token(LET, "let", line: 1, col: 1)`. The lexer does not assign meaning to tokens, it only categorizes them. Meaning is determined by the parser in the next stage, which consumes the token stream and builds an Abstract Syntax Tree.

Tokens are grouped into categories based on the role they play in the language. Keywords are reserved words that have a fixed meaning in the language and cannot be used as identifiers, such as `if`, `else`, `fn`, `while`, and `return`. Identifiers are user-defined names for variables, functions, and other entities. Literals are concrete values written directly in the source code, including integers, floating-point numbers, string literals, and boolean values. Operators define computations and comparisons between values. Delimiters provide structural boundaries that separate expressions, statements, and code blocks. Special tokens like `EOF` signal the end of input and allow the parser to terminate cleanly, while `ILLEGAL` tokens mark characters the lexer cannot recognize and allow error reporting to continue rather than crashing.

The lexer operates on the principle of longest match, also called maximal munch. When multiple token types could match at a given position, the lexer always prefers the longest possible match. This is why `==` is read as a single equality token rather than two separate assignment tokens, and why `<=` is read as a single less-than-or-equal token rather than two separate symbols. This principle is implemented in practice using lookahead, where the lexer peeks at the character immediately following the current one before deciding which token to emit.

Lexical analysis is formally grounded in the theory of regular languages. Every token type can be described by a regular expression, and regular expressions can be compiled into finite automata that recognize them. In practice, most hand-written lexers implement this recognition logic directly in code using character comparison and state variables, which is more efficient and easier to debug than a fully generic automaton-based approach.

## Objectives

The main goal of this laboratory work was to implement a fully functional lexer for a custom mini programming language from scratch. The first objective was to understand what lexical analysis is and how it fits into the broader compilation pipeline, distinguishing the responsibilities of the lexer from those of the parser and evaluator. The second objective was to get deeply familiar with the internal mechanics of a lexer, including character-by-character scanning, one-character lookahead, whitespace skipping, and the mapping of raw text to structured token objects. The third objective was to implement the lexer in Java and demonstrate its correctness across a comprehensive set of test cases that together cover every supported token type. An additional requirement was that the language supported should be non-trivial, meaning it needed to go beyond simple arithmetic and include features such as string literals, boolean values, control flow keywords, function declarations, built-in mathematical functions, compound assignment operators, increment and decrement operators, and multi-line comments.

## Implementation Description

The project was implemented in **Java** and consists of four source files, each with a clearly defined and independent responsibility. The design follows a clean separation where the token type definitions, the token data structure, the scanning logic, and the demonstration code are all kept in separate classes.

### 1. TokenType Enum (`TokenType.java`)

The `TokenType` enum defines every possible category of token the lexer can produce. It is the foundation of the entire system because every other class depends on it to classify and communicate about tokens. The enum is organized into logical groups. The first group covers literals: `INTEGER` for whole numbers, `FLOAT` for decimal numbers, `STRING` for double-quoted text, `BOOLEAN` for the values true and false, and `IDENTIFIER` for user-defined names. The second group covers language keywords: `LET`, `FN`, `RETURN`, `IF`, `ELSE`, `WHILE`, `FOR`, and `PRINT`. The third group covers built-in mathematical functions that the language treats as reserved names: `SIN`, `COS`, `TAN`, `SQRT`, `ABS`, `LOG`, and `POW`. The fourth group covers arithmetic operators: `PLUS`, `MINUS`, `MULTIPLY`, `DIVIDE`, `MODULO`, and `POWER` for the exponentiation operator. The fifth group covers compound assignment operators: `ASSIGN`, `PLUS_ASSIGN`, `MINUS_ASSIGN`, `MUL_ASSIGN`, and `DIV_ASSIGN`. The sixth group covers increment and decrement: `INCREMENT` and `DECREMENT`. The seventh group covers comparison operators: `EQ`, `NOT_EQ`, `LT`, `GT`, `LT_EQ`, and `GT_EQ`. The eighth group covers logical operators: `AND`, `OR`, and `NOT`. The ninth group covers all delimiters: `SEMICOLON`, `COMMA`, `DOT`, `COLON`, `LPAREN`, `RPAREN`, `LBRACE`, `RBRACE`, `LBRACKET`, and `RBRACKET`. Finally the special group contains `COMMENT`, `NEWLINE`, `EOF`, and `ILLEGAL`. Using a dedicated enum rather than plain strings makes the codebase fully type-safe, eliminates the risk of typos in comparisons, and makes it trivial to add new token types in future extensions.

### 2. Token Class (`Token.java`)

The `Token` class is an immutable data structure that bundles four fields together into a single object. The `type` field holds a value from the `TokenType` enum that identifies what category the token belongs to. The `value` field holds the literal string of characters that were read from the source to form this token, which is the lexeme. The `line` and `column` fields hold the exact position in the source file where the token begins. Storing positional information alongside every token is essential because it allows any later stage of the compiler to report precise error messages that point the programmer to the exact line and character that caused a problem, rather than giving a vague or useless error description. The class also overrides `toString()` to produce a consistently formatted output string that makes the token list easy to read during debugging and demonstration.

### 3. Lexer Class (`Lexer.java`)

The `Lexer` class is the core of the implementation and contains all of the scanning logic. It holds four pieces of internal state: the `source` string containing the entire input, a `position` integer that is the index of the currently examined character, a `current` character variable that mirrors `source.charAt(position)` for convenience, and the `line` and `column` integers that track where in the source the scanner currently is. The constructor initializes all of these fields and sets `current` to the first character of the input, or to the null character `\0` if the input is empty.

The public entry point is the `tokenize()` method, which runs a simple loop that repeatedly calls `nextToken()` and appends the result to a list until an `EOF` token is produced. This design gives callers a complete token list in a single method call, which is convenient for testing and demonstration.

The `nextToken()` method acts as the central dispatch function. It begins by calling `skipWhitespaceAndNewlines()` to consume any spaces, tabs, or line breaks that appear between tokens, since whitespace is not significant in this language. It then examines the current character and routes execution to the appropriate specialized reader method. The check for comments comes before the check for the division operator because both start with a slash, and looking ahead at the second character is what distinguishes them.

```java
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
```

The `readNumber()` method accumulates digit characters one by one and watches for a decimal point to determine whether the result should be classified as `INTEGER` or `FLOAT`. It includes a guard against double decimal points by stopping when a second dot is encountered, preventing malformed numbers like `3.1.4` from being silently accepted.

The `readIdentifier()` method accumulates alphanumeric characters and underscores until it reaches a character that cannot be part of an identifier. Once the full word has been collected, it performs a single lookup in a static `HashMap` called `KEYWORDS` that maps every reserved word to its corresponding `TokenType`. If the word is found in the map, that type is used. If it is not found, the method returns an `IDENTIFIER` token. Because the map is built once when the class is loaded and never modified, this lookup is an O(1) operation regardless of how many keywords the language defines.

```java
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
```

The `readString()` method skips the opening double quote and then reads characters one by one until it either finds a closing double quote or reaches the end of the input. It handles escape sequences by detecting a backslash and then examining the following character, translating the pair into the appropriate actual character: `\n` becomes a newline, `\t` becomes a tab, `\\` becomes a single backslash, and `\"` becomes a literal double quote. If the method reaches end of input before finding a closing quote, it returns an `ILLEGAL` token with an explanatory message rather than throwing an exception.

The `readSingleLineComment()` method skips the two opening slashes and then reads characters until it reaches a newline or end of input. The `readMultiLineComment()` method skips the opening `/*` and reads until it finds the closing `*/` sequence, correctly handling comments that span multiple lines.

The `readOperatorOrDelimiter()` method uses a switch statement on the consumed character and calls `peek()` before each compound token decision. The `peek()` method reads the character at `position + 1` without moving the pointer, implementing a lookahead of exactly one character which is sufficient to resolve all ambiguities in this language.

```java
private Token readOperatorOrDelimiter() {
    int startCol = column;
    char ch = current;
    advance();
    switch (ch) {
        case '+':
            if (current == '+') { advance(); return new Token(TokenType.INCREMENT,   "++", line, startCol); }
            if (current == '=') { advance(); return new Token(TokenType.PLUS_ASSIGN, "+=", line, startCol); }
            return new Token(TokenType.PLUS, "+", line, startCol);
        // ... remaining cases
    }
}
```

The `advance()` method is responsible for moving the scanner forward. It increments `position` and updates `current` to the next character. Crucially, it also updates the `line` and `column` counters: when it consumes a newline character it increments `line` and resets `column` to one, otherwise it simply increments `column`. This tracking happens automatically with every character consumed, so the line and column stored in each produced token always reflect the start position of that token accurately.

### 4. Main Class (`Main.java`)

The `Main` class serves as the demonstration and testing harness for the lexer. It defines ten test cases as inline source strings and runs each one through the lexer, printing the full token list to the console in a formatted table. Each test is wrapped in a helper method called `runTest()` that prints the title, the source code, and then each token returned by `tokenize()`. Tokens of type `ILLEGAL` are flagged with a warning marker and tokens of type `COMMENT` are marked to make them visually distinguishable in the output. The ten tests cover variable declarations with all literal types, arithmetic expressions with mathematical functions, comparison and logical operators, compound assignment and increment operators, function declarations and calls, if-else control flow, while loops, single-line and multi-line comments, a complete recursive Fibonacci function, and deliberate error input with illegal characters and an unterminated string.

## Challenges and Solutions

The first challenge encountered during development was handling the overlap between operators that share a common prefix. The single character `=` is the assignment operator, but `==` is the equality comparison. Similarly, `+` is addition but `++` is increment and `+=` is compound assignment. Reading only one character at a time would cause the lexer to always emit the shorter single-character token, which would be incorrect. The solution was to implement the `peek()` method that reads one character ahead without consuming it. Before emitting any single-character operator token, the method first checks whether the next character forms a longer valid compound token, and if so it consumes the extra character and emits the compound version instead. This lookahead of one character is sufficient to resolve all ambiguities present in the language.

A second challenge was distinguishing a comment starting with `//` or `/*` from a plain division operator `/`. Both begin with the same character, so the lexer cannot make a decision based on the current character alone. The solution was to check `peek()` at the very beginning of `nextToken()`, before falling through to the operator handler. If the current character is `/` and the next character is also `/`, the scanner routes to `readSingleLineComment()`. If the next character is `*`, it routes to `readMultiLineComment()`. Only if neither condition holds does the `/` get treated as a division operator.

A third challenge was correctly tracking line and column numbers across the entire input, including inside multi-line comments and string literals that may span multiple lines. Because all character consumption in the lexer goes through the single `advance()` method, the solution was to place the line and column update logic entirely inside `advance()`. Every time `advance()` sees a newline character it increments the line counter and resets the column, and on any other character it simply increments the column. This centralized approach means that no matter which reader method is currently active, the position tracking is always correct and consistent.

A fourth challenge was making the lexer resilient to invalid input. A naive implementation might throw an exception when it sees an unrecognized character or an unterminated string, which would abort the entire scan and prevent the compiler from reporting any further problems. Instead, the lexer wraps every error case in an `ILLEGAL` token and continues scanning normally. This means a single pass over the source can surface multiple problems simultaneously, which is a significant improvement in usability for the programmer using the language.

## Results and Analysis

The lexer was validated against ten test cases that together exercise every token category defined in the enum. The test for variable declarations confirms that `LET`, `IDENTIFIER`, `ASSIGN`, `INTEGER`, `FLOAT`, `STRING`, and `BOOLEAN` tokens are all produced with correct values and precise line and column positions. The mathematical functions test confirms that `sin`, `cos`, `sqrt`, `pow`, `abs`, `log`, and `tan` are recognized as dedicated token types and distinguished from plain user-defined identifiers. This is important because it means the parser and evaluator can handle them specially without needing to look up their names in a symbol table.

The comparison and logical operator test verifies that `==`, `!=`, `<=`, `>=`, `&&`, and `||` are each correctly identified as single two-character tokens rather than being split into pairs of single-character tokens. The compound assignment test confirms that `+=`, `-=`, `*=`, and `/=` are similarly handled, along with `++` and `--` as distinct increment and decrement tokens and `**` as the power operator.

The function declaration test with the `add` function and the Fibonacci test together confirm that the lexer handles realistic, multi-line, nested program structures correctly. The Fibonacci test in particular exercises recursive function calls, nested if-else blocks, and multiple levels of parentheses, all of which the lexer processes without any issues because it operates purely at the character level and has no notion of nesting or structure.

The comment test demonstrates that both single-line comments starting with `//` and multi-line comments delimited by `/*` and `*/` are captured as `COMMENT` tokens with their trimmed text content preserved. The error handling test confirms that each illegal character produces exactly one `ILLEGAL` token, that scanning resumes correctly after each illegal character, and that an unterminated string produces an `ILLEGAL` token with a descriptive message rather than causing a crash or silent failure.

## Conclusion

In this laboratory work I successfully designed and implemented a fully functional lexer for a custom mini programming language in Java. The implementation correctly handles all standard token categories including keywords, user-defined identifiers, integer and floating-point numeric literals, double-quoted string literals with escape sequences, boolean literals, the complete set of arithmetic and logical operators including all compound forms, built-in mathematical function names, single-line and multi-line comments, and all standard delimiter characters. Line and column tracking was integrated directly into the character advancement logic, ensuring that every token carries accurate positional information. The error handling strategy of emitting `ILLEGAL` tokens rather than throwing exceptions allows the lexer to report multiple problems in a single pass and makes the overall system more robust. This implementation forms a complete and solid foundation that could be directly extended with a parser in the next stage of the project to build a fully working language interpreter.

## References

Crafting Interpreters, Robert Nystrom, https://craftinginterpreters.com

LLVM Kaleidoscope Tutorial, https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl01.html

Compilers: Principles, Techniques, and Tools, Aho, Lam, Sethi, Ullman

Course slides: Lexer / Scanner, Vasile Drumea, Irina Cojuhari