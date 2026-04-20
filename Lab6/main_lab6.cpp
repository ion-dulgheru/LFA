#include "lexer.h"
#include "parser.h"
#include <iostream>

void separator(const std::string& title) {
  std::cout << "  " << title << "\n";
}

void runTest(const std::string& label, const std::string& source) {
    separator(label);
    std::cout << "Source:\n" << source << "\n";

    // ── Tokenise ──
    std::cout << "\n--- TOKENS ---\n";
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();
    for (const auto& tok : tokens)
        if (tok.type != TokenType::EOF_TOKEN)
            std::cout << "  " << tok.toString() << "\n";

    // ── Parse ──
    std::cout << "\n--- AST ---\n";
    Parser parser(tokens);
    auto program = parser.parse();
    program->print(1);
}

int main() {
    std::cout << "  Parser & AST   Lab 6\n";
    std::cout << "  Formal Languages & Finite Automata\n";

    // ── Test 1: let declarations + arithmetic with ** and % ──────
    runTest("TEST 1: Let declarations & arithmetic", R"(
// compute some values
let x = 10;
let y = 3 + 5 * 2;
let z = x ** 2 % 7;
print(z);
)");

    // ── Test 2: compound assignment + increment/decrement ─────────
    runTest("TEST 2: Compound assignment & increment", R"(
let counter = 0;
counter += 5;
counter -= 2;
counter *= 3;
counter++;
print(counter);
)");

    // ── Test 3: if / else with logical operators ──────────────────
    runTest("TEST 3: If-else with logical operators", R"(
let score = 85;
let passed = score >= 60 && score <= 100;
if (passed) {
    print("pass");
} else {
    print("fail");
}
)");

    // ── Test 4: while loop ────────────────────────────────────────
    runTest("TEST 4: While loop", R"(
let i = 0;
let sum = 0;
while (i < 5) {
    sum += i;
    i++;
}
print(sum);
)");

    // ── Test 5: for loop ──────────────────────────────────────────
    runTest("TEST 5: For loop", R"(
let result = 0;
for (let i = 0; i < 10; i++) {
    result += i;
}
print(result);
)");

    // ── Test 6: math built-ins ────────────────────────────────────
    runTest("TEST 6: Math built-ins", R"(
let angle = 1;
let s = sin(angle);
let c = cos(angle);
let r = sqrt(pow(s, 2) + pow(c, 2));
print(r);
)");

    // ── Test 7: function definition + return + nested constructs ──
    runTest("TEST 7: Function definition & nested constructs", R"(
fn factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n);
}
let val = 5;
let res = abs(val);
print(res);
)");


    return 0;
}