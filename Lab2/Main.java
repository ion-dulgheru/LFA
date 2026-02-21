package Lab2;
import java.util.*;

public class Main {
    public static void main(String[] args) {
        System.out.println("=== Lab 2: Variant 10 ===");

        // Definim starile si tranzitiile pentru Varianta 10
        Set<String> states = new HashSet<>(Arrays.asList("q0", "q1", "q2", "q3"));
        String startState = "q0";
        Set<String> finalStates = new HashSet<>(Collections.singletonList("q3"));
        
        Map<String, Map<Character, Set<String>>> transitions = new HashMap<>();
        transitions.put("q0", new HashMap<>());
        transitions.get("q0").put('a', new HashSet<>(Collections.singletonList("q1")));
        transitions.get("q0").put('b', new HashSet<>(Collections.singletonList("q2")));

        transitions.put("q1", new HashMap<>());
        transitions.get("q1").put('b', new HashSet<>(Arrays.asList("q1", "q2"))); // Aici e nedeterminismul

        transitions.put("q2", new HashMap<>());
        transitions.get("q2").put('c', new HashSet<>(Collections.singletonList("q3")));

        transitions.put("q3", new HashMap<>());
        transitions.get("q3").put('a', new HashSet<>(Collections.singletonList("q1")));

        FiniteAutomaton ndfa = new FiniteAutomaton(states, startState, finalStates, transitions);

        System.out.println("\n1. Este Automatul determinist?");
        System.out.println("Raspuns: " + (ndfa.isDeterministic() ? "DA (DFA)" : "NU (NDFA)"));

        System.out.println("\n2. Conversie Automatul in Gramatica:");
        Grammar convertedGrammar = ndfa.toRegularGrammar();
        convertedGrammar.printRules();

        System.out.println("\n3. Clasificare Chomsky pentru noua Gramatica:");
        System.out.println(convertedGrammar.classifyChomsky());

        System.out.println("\n4. Conversie NDFA in DFA:");
        FiniteAutomaton dfa = ndfa.toDFA();
        dfa.printAutomaton();
        
        System.out.println("\nEste noul Automat determinist? " + (dfa.isDeterministic() ? "DA" : "NU"));
    }
}