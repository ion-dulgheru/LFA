import java.util.*;

public class RegexGenerator {

    private static final Random RAND    = new Random();
    private static final int    MAX_REP = 5;


    abstract static class Node {
        abstract String generate();
        abstract String describe(int depth);
        String pad(int d) { return "  ".repeat(d); }
    }

    static class Literal extends Node {
        final char ch;
        Literal(char ch) { this.ch = ch; }

        @Override String generate() { return String.valueOf(ch); }

        @Override String describe(int d) {
            return pad(d) + "Literal('" + ch + "')";
        }
    }

    static class Sequence extends Node {
        final List<Node> parts;
        Sequence(List<Node> parts) { this.parts = parts; }

        @Override String generate() {
            StringBuilder sb = new StringBuilder();
            for (Node n : parts) sb.append(n.generate());
            return sb.toString();
        }

        @Override String describe(int d) {
            StringBuilder sb = new StringBuilder(pad(d) + "Sequence[\n");
            for (Node n : parts) sb.append(n.describe(d + 1)).append("\n");
            return sb.append(pad(d) + "]").toString();
        }
    }

    static class Alternation extends Node {
        final List<Node> choices;
        Alternation(List<Node> choices) { this.choices = choices; }

        @Override String generate() {
            return choices.get(RAND.nextInt(choices.size())).generate();
        }

        @Override String describe(int d) {
            StringBuilder sb = new StringBuilder(pad(d) + "Alternation[\n");
            for (Node c : choices) sb.append(c.describe(d + 1)).append("\n");
            return sb.append(pad(d) + "]").toString();
        }
    }

    static class Repeat extends Node {
        final Node   inner;
        final int    min, max;
        final String sym;

        Repeat(Node inner, int min, int max, String sym) {
            this.inner = inner;
            this.min   = min;
            this.max   = max;
            this.sym   = sym;
        }

        @Override String generate() {
            int count = (min == max) ? min : min + RAND.nextInt(max - min + 1);
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < count; i++) sb.append(inner.generate());
            return sb.toString();
        }

        @Override String describe(int d) {
            return pad(d) + "Repeat" + sym + "[" + min + ".." + max + "](\n"
                 + inner.describe(d + 1) + "\n" + pad(d) + ")";
        }
    }

    private String src;
    private int    pos;
    private final List<String> steps = new ArrayList<>();

    public Node parse(String regex) {
        this.src = regex;
        this.pos = 0;
        steps.clear();
        steps.add("START  parsing: \"" + regex + "\"");
        Node result = parseSequence();
        steps.add("DONE   AST built successfully");
        return result;
    }

    private Node parseSequence() {
        List<Node> parts = new ArrayList<>();
        while (pos < src.length()
               && src.charAt(pos) != ')'
               && src.charAt(pos) != '|') {
            parts.add(parseAtom());
        }
        if (parts.size() == 1) return parts.get(0);
        steps.add("BUILD  Sequence with " + parts.size() + " elements");
        return new Sequence(parts);
    }

    private Node parseAtom() {
        Node node;
        char ch = src.charAt(pos);

        if (ch == '(') {
            pos++;
            steps.add("OPEN   group at pos " + pos);
            List<Node> alts = new ArrayList<>();
            alts.add(parseSequence());
            while (pos < src.length() && src.charAt(pos) == '|') {
                pos++;
                alts.add(parseSequence());
            }
            pos++;
            node = (alts.size() == 1) ? alts.get(0) : new Alternation(alts);
            steps.add("CLOSE  group → " + alts.size() + " alternative(s)");
        } else {
            node = new Literal(ch);
            steps.add("READ   literal '" + ch + "'");
            pos++;
        }

        if (pos < src.length()) {
            char q = src.charAt(pos);
            if (q == '?') {
                steps.add("QUANT  '?' (0 or 1)");
                pos++;
                return new Repeat(node, 0, 1, "?");
            }
            if (q == '*') {
                steps.add("QUANT  '*' (0.." + MAX_REP + ")");
                pos++;
                return new Repeat(node, 0, MAX_REP, "*");
            }
            if (q == '+' || q == '\u207A') {
                steps.add("QUANT  '+' (1.." + MAX_REP + ")");
                pos++;
                return new Repeat(node, 1, MAX_REP, "+");
            }
            if (q == '\u00B2') {
                steps.add("QUANT  '²' (exactly 2)");
                pos++;
                return new Repeat(node, 2, 2, "²");
            }
            if (q == '\u00B3') {
                steps.add("QUANT  '³' (exactly 3)");
                pos++;
                return new Repeat(node, 3, 3, "³");
            }
            if (q >= '2' && q <= '9') {
                int n = q - '0';
                steps.add("QUANT  '" + n + "' (exactly " + n + ")");
                pos++;
                return new Repeat(node, n, n, String.valueOf(n));
            }
        }

        return node;
    }

    public List<String> getSteps() { return Collections.unmodifiableList(steps); }

    public static void main(String[] args) {

        String[] regexes = {
            "M?N\u00B2(O|P)\u00B3Q*R\u207A",
            "(X|Y|Z)\u00B38\u207A(9|0)",
            "(H|I)(J|K)L*N?"
        };

        String[] labels = {
    "M?N^2(O|P)^3Q*R+",
    "(X|Y|Z)^3 8+(9|0)",
    "(H|I)(J|K)L*N?"
};

        RegexGenerator gen = new RegexGenerator();

        for (int i = 0; i < regexes.length; i++) {
            System.out.println("  Regex " + (i + 1) + ": " + labels[i]);

            Node root = gen.parse(regexes[i]);

            System.out.println("\n[Bonus] Processing steps:");
            List<String> steps = gen.getSteps();
            for (int s = 0; s < steps.size(); s++) {
                System.out.printf("  %2d. %s%n", s + 1, steps.get(s));
            }

            System.out.println("\n[AST] Tree structure:");
            System.out.println(root.describe(1));

            System.out.println("\n[Generated] 10 valid strings:");
            Set<String> seen = new LinkedHashSet<>();
            int attempts = 0;
            while (seen.size() < 10 && attempts < 200) {
                seen.add(root.generate());
                attempts++;
            }
            for (String s : seen) System.out.println("  → " + s);

            System.out.println();
        }
    }
}