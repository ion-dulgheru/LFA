import java.util.List;

public class Main {

    public static void main(String[] args) {

        runTest("Test 1: Variabile",
            "let varsta = 25;\n" +
            "let nume = \"Alex\";\n" +
            "let pi = 3.14159;\n" +
            "let valid = true;"
        );

        runTest("Test 2: Aritmetica si chestii de matematica",
            "let rezultat = (10 + 5) * 2 - 3 / 1.5;\n" +
            "let s = sin(3.14);\n" +
            "let c = cos(0.0);\n" +
            "let r = sqrt(pow(3, 2) + pow(4, 2));\n" +
            "let x = abs(-42) + log(100);"
        );

        runTest("Test 3: Operatori logici si comparare",
            "let a = 10;\n" +
            "let b = 20;\n" +
            "let check = (a == b) || (a != b) && (a <= b) || (b >= a);"
        );

        runTest("Test 4: Incrementare / Decrementare",
            "let contor = 0;\n" +
            "contor += 5;\n" +
            "contor -= 2;\n" +
            "contor *= 3;\n" +
            "contor /= 2;\n" +
            "contor++;\n" +
            "contor--;"
        );

        runTest("Test 5: Functii",
            "fn adunare(a, b) {\n" +
            "    return a + b;\n" +
            "}\n" +
            "let suma = adunare(10, 20);\n" +
            "print(suma);"
        );

        runTest("Test 6: Structura de decizie (if / else)",
            "if (x > 0) {\n" +
            "    print(\"pozitiv\");\n" +
            "} else {\n" +
            "    print(\"negativ\");\n" +
            "}"
        );

        runTest("Test 7: Bucla while",
            "let i = 0;\n" +
            "while (i < 10) {\n" +
            "    i++;\n" +
            "}"
        );

        runTest("Test 8: Comentarii inline si multiline",
            "// un comentariu simplu pe o linie\n" +
            "let x = 42; // inca un comentariu pe aceeasi linie\n" +
            "/* aici avem\n" +
            "   un comentariu pe\n" +
            "   mai multe linii */\n" +
            "let y = x ** 2;"
        );

        runTest("Test 9: Program cu Fibonacci",
            "fn fibonacci(n) {\n" +
            "    if (n == 0) {\n" +
            "        return 0;\n" +
            "    } else {\n" +
            "        if (n == 1) {\n" +
            "            return 1;\n" +
            "        } else {\n" +
            "            return fibonacci(n - 1) + fibonacci(n - 2);\n" +
            "        }\n" +
            "    }\n" +
            "}\n" +
            "let rez = fibonacci(10);\n" +
            "print(rez);"
        );

        runTest("Test 10: Eroare cu caractere ilegale",
            "let x = @#$;\n" +
            "let gresit = \"string neterminat"
        );
    }

    private static void runTest(String titlu, String sursa) {
        
        System.out.println("=== " + titlu + " ===");
        System.out.println("Cod Sursa:");
        
        // Afisez codul sursa asa cum e primit din string
        for (String linie : sursa.split("\n")) {
            System.out.println("  " + linie);
        }
        
        System.out.println("Tokens generate:");

        // Fac initializarea lexer-ului
        Lexer lexer = new Lexer(sursa);
        List<Token> tokeni = lexer.tokenize();

        // Parcurg lista de tokeni si adaug markere pt a gasi usor erorile si comentariile
        for (Token t : tokeni) {
            String indicator = "";
            if (t.type == TokenType.ILLEGAL) {
                indicator = " EROARE ILEGALA";
            } else if (t.type == TokenType.COMMENT) {
                indicator = " COMENTARIU";
            }
            
            System.out.println("  " + t + indicator);
        }

        System.out.println(); 
    }
}
