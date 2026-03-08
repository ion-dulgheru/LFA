package Lab2;
import java.util.*;

public class Grammar {
    private Map<String, List<String>> rules;
    private static final Random random = new Random();

    // Constructorul tău original pentru Lab 1
    public Grammar() {
        rules = new HashMap<>();
        rules.put("S", Arrays.asList("aB"));
        rules.put("B", Arrays.asList("bB", "cL"));
        rules.put("L", Arrays.asList("cL", "aS", "b"));
    }

    // Constructor nou pentru Lab 2 (pentru a crea o gramatică direct din FA)
    public Grammar(Map<String, List<String>> rules) {
        this.rules = rules;
    }

    public String generateString() {
        StringBuilder word = new StringBuilder();
        String currentState = "S";
        
        while (currentState.length() > 0) {
            List<String> options = rules.get(currentState);
            if (options == null) break;

            String chosenRule = options.get(random.nextInt(options.size()));
            if (chosenRule.length() > 1) {
                char symbol = chosenRule.charAt(0);
                String nextState = chosenRule.substring(1);
                word.append(symbol);
                currentState = nextState;
            } else {
                word.append(chosenRule);
                if(chosenRule.equals("ε")) {
                    word.setLength(word.length() - 1); // ștergem epsilon dacă apare
                }
                currentState = "";
            }
        }
        return word.toString();
    }

    // Task Lab 2: Clasificarea Ierarhiei Chomsky
    public String classifyChomsky() {
        boolean isRegular = true;
        boolean isContextFree = true;
        boolean isContextSensitive = true;

        for (Map.Entry<String, List<String>> entry : rules.entrySet()) {
            String lhs = entry.getKey();
            for (String rhs : entry.getValue()) {
                // Tip 1: |LHS| <= |RHS| (cu excepția lui epsilon)
                if (lhs.length() > rhs.length() && !rhs.equals("ε")) isContextSensitive = false;
                
                // Tip 2: LHS trebuie să fie exact un neterminal (o literă mare)
                if (lhs.length() != 1 || !Character.isUpperCase(lhs.charAt(0))) isContextFree = false;
                
                // Tip 3: A -> aB sau A -> a
                if (lhs.length() == 1 && Character.isUpperCase(lhs.charAt(0))) {
                    if (rhs.length() > 2) {
                        isRegular = false;
                    } else if (rhs.length() == 2 && (!Character.isLowerCase(rhs.charAt(0)) || !Character.isUpperCase(rhs.charAt(1)))) {
                        isRegular = false;
                    } else if (rhs.length() == 1 && (!Character.isLowerCase(rhs.charAt(0)) && !rhs.equals("ε"))) {
                        isRegular = false;
                    }
                }
            }
        }

        if (isRegular) return "Type 3: Regular Grammar";
        if (isContextFree) return "Type 2: Context-Free Grammar";
        if (isContextSensitive) return "Type 1: Context-Sensitive Grammar";
        return "Type 0: Unrestricted Grammar";
    }

    public void printRules() {
        for (Map.Entry<String, List<String>> entry : rules.entrySet()) {
            System.out.println(entry.getKey() + " -> " + String.join(" | ", entry.getValue()));
        }
    }
}