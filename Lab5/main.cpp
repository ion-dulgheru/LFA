#include "grammar.h"
#include <iostream>

int main() {
    std::cout << "  CNF Normalisation   Variant 10\n";
    std::cout << "  Formal Languages & Finite Automata\n";

    std::set<std::string> VN = {"S", "A", "B", "D"};
    std::set<std::string> VT = {"a", "b", "d"};
    std::string           SS = "S";

    PMap P;
    P["S"] = {{"d","B"}, {"A","B"}};
    P["A"] = {{"d"}, {"d","S"}, {"a","A","a","A","b"}, {}};
    P["B"] = {{"a"}, {"a","S"}, {"A"}};
    P["D"] = {{"A","b","a"}};

    Grammar g(VN, VT, P, SS);
    g.toCNF();


    return 0;
}
