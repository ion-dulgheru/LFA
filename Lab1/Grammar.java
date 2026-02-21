package Lab1;
import java.util.*;
public class Grammar
{
      private Map<String,List<String>> rules;
      private static final Random random = new Random();

    public Grammar()
    {
      rules = new HashMap<>();
      rules.put("S",Arrays.asList("aB"));
            rules.put("B",Arrays.asList("bB","cL"));
      rules.put("L",Arrays.asList("cL","aS","b"));
    }
    public String generateString()
    {
        StringBuilder word = new StringBuilder(); 
        String currentState = "S";
        System.out.print(currentState + "-> ");
        while (currentState.length() > 0 ){
            List<String> options = rules.get(currentState);
            if (options == null ) break;

            String chosenRule = options.get(random.nextInt(options.size()));
                  if(chosenRule.length() > 1){
                    char symbol = chosenRule.charAt(0);
                    String nextState = chosenRule.substring(1);
                    word.append(symbol);
                    currentState = nextState;
                    System.out.print(symbol + nextState +  "-> ");
                  } else {
                    word.append(chosenRule);
                    System.out.print(chosenRule +  " ");
                    currentState = "";
                  }

        }

        return word.toString();
    }

    public FiniteAutomaton toFiniteAutomaton() {
      
        Set<String> states = new HashSet<>(rules.keySet());
        states.add("X"); 
        
        Map<String, Map<Character, String>> transitions = new HashMap<>();

        for (String state : rules.keySet()) {
            transitions.putIfAbsent(state, new HashMap<>());
            
            for (String production : rules.get(state)) {
                char input = production.charAt(0);
                String nextState;

                if (production.length() > 1) {
                    nextState = production.substring(1);
                } else {
                    nextState = "X";
                }
                transitions.get(state).put(input, nextState);
            }
        }

        return new FiniteAutomaton(states, "S", "X", transitions);
    }
}