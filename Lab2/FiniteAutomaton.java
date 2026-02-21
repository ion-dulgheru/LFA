package Lab2;
import java.util.*;

public class FiniteAutomaton {
    private Set<String> states;
    private String startState;
    private Set<String> finalStates;
    private Map<String, Map<Character, Set<String>>> transitions;

    public FiniteAutomaton(Set<String> states, String startState, Set<String> finalStates, Map<String, Map<Character, Set<String>>> transitions) {
        this.states = states;
        this.startState = startState;
        this.finalStates = finalStates;
        this.transitions = transitions;
    }

    // Task Lab 2: Verificare Determinism
    public boolean isDeterministic() {
        for (Map<Character, Set<String>> stateTransitions : transitions.values()) {
            for (Set<String> destinations : stateTransitions.values()) {
                if (destinations.size() > 1) {
                    return false; // Dacă există o bifurcație, e NDFA
                }
            }
        }
        return true;
    }

    // Task Lab 2: Conversie FA -> Gramatică Regulată
    public Grammar toRegularGrammar() {
        Map<String, List<String>> grammarRules = new HashMap<>();

        for (String state : states) {
            grammarRules.put(state, new ArrayList<>());
            if (transitions.containsKey(state)) {
                for (Map.Entry<Character, Set<String>> transition : transitions.get(state).entrySet()) {
                    char symbol = transition.getKey();
                    for (String destState : transition.getValue()) {
                        grammarRules.get(state).add(symbol + destState);
                    }
                }
            }
            // Dacă e stare finală, adăugăm epsilon
            if (finalStates.contains(state)) {
                grammarRules.get(state).add("ε");
            }
        }
        return new Grammar(grammarRules);
    }

    // Task Lab 2: Conversie NDFA -> DFA (Subset Construction)
    public FiniteAutomaton toDFA() {
        if (this.isDeterministic()) return this;

        Set<Set<String>> dfaStates = new HashSet<>();
        Queue<Set<String>> queue = new LinkedList<>();
        Map<Set<String>, Map<Character, Set<String>>> dfaTransitions = new HashMap<>();
        Set<Set<String>> dfaFinalStates = new HashSet<>();

        Set<String> initialDFAState = new HashSet<>();
        initialDFAState.add(this.startState);
        
        queue.add(initialDFAState);
        dfaStates.add(initialDFAState);

        // Determinăm alfabetul
        Set<Character> alphabet = new HashSet<>();
        for (Map<Character, Set<String>> paths : transitions.values()) {
            alphabet.addAll(paths.keySet());
        }

        while (!queue.isEmpty()) {
            Set<String> currentDFAState = queue.poll();
            dfaTransitions.putIfAbsent(currentDFAState, new HashMap<>());

            // Verificăm dacă această stare macro conține o stare finală veche
            for (String subState : currentDFAState) {
                if (this.finalStates.contains(subState)) {
                    dfaFinalStates.add(currentDFAState);
                    break;
                }
            }

            for (char symbol : alphabet) {
                Set<String> nextDFAState = new HashSet<>();
                for (String subState : currentDFAState) {
                    if (transitions.containsKey(subState) && transitions.get(subState).containsKey(symbol)) {
                        nextDFAState.addAll(transitions.get(subState).get(symbol));
                    }
                }

                if (!nextDFAState.isEmpty()) {
                    dfaTransitions.get(currentDFAState).put(symbol, nextDFAState);
                    if (!dfaStates.contains(nextDFAState)) {
                        dfaStates.add(nextDFAState);
                        queue.add(nextDFAState);
                    }
                }
            }
        }

        return formatDfa(dfaStates, initialDFAState, dfaFinalStates, dfaTransitions);
    }

    // Helper pentru a transforma Set-urile înapoi în String-uri pentru DFA-ul nou (ex: [q1, q2] -> "q1q2")
    private FiniteAutomaton formatDfa(Set<Set<String>> states, Set<String> start, Set<Set<String>> finals, Map<Set<String>, Map<Character, Set<String>>> transitions) {
        Set<String> newStates = new HashSet<>();
        Set<String> newFinals = new HashSet<>();
        String newStart = formatStateName(start);
        Map<String, Map<Character, Set<String>>> newTransitions = new HashMap<>();

        for (Set<String> s : states) newStates.add(formatStateName(s));
        for (Set<String> f : finals) newFinals.add(formatStateName(f));

        for (Map.Entry<Set<String>, Map<Character, Set<String>>> entry : transitions.entrySet()) {
            String fromState = formatStateName(entry.getKey());
            newTransitions.put(fromState, new HashMap<>());
            for (Map.Entry<Character, Set<String>> trans : entry.getValue().entrySet()) {
                Set<String> toState = new HashSet<>();
                toState.add(formatStateName(trans.getValue()));
                newTransitions.get(fromState).put(trans.getKey(), toState);
            }
        }
        return new FiniteAutomaton(newStates, newStart, newFinals, newTransitions);
    }

    private String formatStateName(Set<String> stateSet) {
        List<String> list = new ArrayList<>(stateSet);
        Collections.sort(list); // sortăm ca "q1, q2" să fie mereu "q1q2"
        return "S_(" + String.join(",", list) + ")";
    }

    public void printAutomaton() {
        System.out.println("States: " + states);
        System.out.println("Start State: " + startState);
        System.out.println("Final States: " + finalStates);
        System.out.println("Transitions:");
        for (String state : transitions.keySet()) {
            for (Map.Entry<Character, Set<String>> trans : transitions.get(state).entrySet()) {
                System.out.println("  " + state + " --" + trans.getKey() + "--> " + trans.getValue());
            }
        }
    }
}