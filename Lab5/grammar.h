#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <iostream>
#include <sstream>

using RHS  = std::vector<std::string>;
using PMap = std::map<std::string, std::vector<RHS>>;

class Grammar {
public:
    std::set<std::string> VN;
    std::set<std::string> VT;
    PMap                  P;
    std::string           S;
    int                   freshCounter = 0;

    Grammar() = default;
    Grammar(std::set<std::string> vn,
            std::set<std::string> vt,
            PMap p,
            std::string s)
        : VN(std::move(vn)), VT(std::move(vt)),
          P(std::move(p)),   S(std::move(s)) {}

    std::string freshNT(const std::string& prefix = "X") {
        while (true) {
            std::string name = prefix + std::to_string(++freshCounter);
            if (!VN.count(name) && !VT.count(name)) {
                VN.insert(name);
                return name;
            }
        }
    }

    void addProduction(const std::string& lhs, const RHS& rhs) {
        auto& vec = P[lhs];
        if (std::find(vec.begin(), vec.end(), rhs) == vec.end())
            vec.push_back(rhs);
    }

    bool isNonTerminal(const std::string& s) const { return VN.count(s) > 0; }
    bool isTerminal   (const std::string& s) const { return VT.count(s) > 0; }

    void print(const std::string& title = "") const {
        if (!title.empty()) {
            std::cout << "  " << title << "\n";
        }
        std::cout << "  VN = { ";
        for (const auto& s : VN) std::cout << s << " ";
        std::cout << "}\n";
        std::cout << "  VT = { ";
        for (const auto& s : VT) std::cout << s << " ";
        std::cout << "}\n";
        std::cout << "  S  = " << S << "\n";
        std::cout << "  Productions:\n";
        for (const auto& [lhs, rhsList] : P) {
            for (const auto& rhs : rhsList) {
                std::cout << "    " << lhs << " -> ";
                if (rhs.empty()) std::cout << "eps";
                else for (const auto& sym : rhs) std::cout << sym;
                std::cout << "\n";
            }
        }
    }

    std::set<std::string> findNullable() const {
        std::set<std::string> nullable;
        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto& [lhs, rhsList] : P) {
                for (const auto& rhs : rhsList) {
                    bool allNull = true;
                    for (const auto& sym : rhs)
                        if (!nullable.count(sym)) { allNull = false; break; }
                    if ((rhs.empty() || allNull) && !nullable.count(lhs)) {
                        nullable.insert(lhs);
                        changed = true;
                    }
                }
            }
        }
        return nullable;
    }

    void eliminateEpsilon() {
        std::set<std::string> nullable = findNullable();

        std::cout << "  Nullable symbols: { ";
        for (const auto& s : nullable) std::cout << s << " ";
        std::cout << "}\n";

        PMap newP;
        for (const auto& [lhs, rhsList] : P) {
            std::set<RHS> seen;
            for (const auto& rhs : rhsList) {
                if (rhs.empty()) continue;

                std::vector<int> nullPos;
                for (int i = 0; i < (int)rhs.size(); ++i)
                    if (nullable.count(rhs[i])) nullPos.push_back(i);

                int sz = (int)nullPos.size();
                for (int mask = 0; mask < (1 << sz); ++mask) {
                    RHS candidate;
                    for (int i = 0; i < (int)rhs.size(); ++i) {
                        bool omit = false;
                        for (int b = 0; b < sz; ++b)
                            if ((mask >> b & 1) && nullPos[b] == i) { omit = true; break; }
                        if (!omit) candidate.push_back(rhs[i]);
                    }
                    if (!candidate.empty() && !seen.count(candidate)) {
                        seen.insert(candidate);
                        newP[lhs].push_back(candidate);
                    }
                }
            }
        }
        P = newP;
    }

    void eliminateUnitProductions() {
        std::set<std::pair<std::string,std::string>> unitPairs;
        for (const auto& nt : VN) unitPairs.insert({nt, nt});

        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto& [x, y] : unitPairs) {
                for (const auto& rhs : P[y]) {
                    if (rhs.size() == 1 && isNonTerminal(rhs[0])) {
                        auto pr = std::make_pair(x, rhs[0]);
                        if (!unitPairs.count(pr)) { unitPairs.insert(pr); changed = true; }
                    }
                }
            }
        }

        PMap newP;
        for (const auto& [A, B] : unitPairs) {
            if (A == B) continue;
            for (const auto& rhs : P[B]) {
                bool isUnit = (rhs.size() == 1 && isNonTerminal(rhs[0]));
                if (!isUnit) {
                    auto& vec = newP[A];
                    if (std::find(vec.begin(), vec.end(), rhs) == vec.end())
                        vec.push_back(rhs);
                }
            }
        }
        for (const auto& [lhs, rhsList] : P) {
            for (const auto& rhs : rhsList) {
                bool isUnit = (rhs.size() == 1 && isNonTerminal(rhs[0]));
                if (!isUnit) {
                    auto& vec = newP[lhs];
                    if (std::find(vec.begin(), vec.end(), rhs) == vec.end())
                        vec.push_back(rhs);
                }
            }
        }
        P = newP;
    }

    void eliminateInaccessible() {
        std::set<std::string> reachable;
        std::vector<std::string> queue = {S};
        while (!queue.empty()) {
            std::string cur = queue.back(); queue.pop_back();
            if (reachable.count(cur)) continue;
            reachable.insert(cur);
            for (const auto& rhs : P[cur])
                for (const auto& sym : rhs)
                    if (isNonTerminal(sym) && !reachable.count(sym))
                        queue.push_back(sym);
        }

        std::cout << "  Reachable: { ";
        for (const auto& s : reachable) std::cout << s << " ";
        std::cout << "}\n";

        std::set<std::string> toRemove;
        for (const auto& nt : VN)
            if (!reachable.count(nt)) toRemove.insert(nt);

        for (const auto& nt : toRemove) { VN.erase(nt); P.erase(nt); }
    }

    void eliminateNonProductive() {
        std::set<std::string> productive;
        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto& [lhs, rhsList] : P) {
                for (const auto& rhs : rhsList) {
                    bool ok = true;
                    for (const auto& sym : rhs)
                        if (!isTerminal(sym) && !productive.count(sym)) { ok = false; break; }
                    if (ok && !productive.count(lhs)) { productive.insert(lhs); changed = true; }
                }
            }
        }

        std::cout << "  Productive: { ";
        for (const auto& s : productive) std::cout << s << " ";
        std::cout << "}\n";

        std::set<std::string> toRemove;
        for (const auto& nt : VN)
            if (!productive.count(nt)) toRemove.insert(nt);

        for (const auto& nt : toRemove) { VN.erase(nt); P.erase(nt); }

        PMap newP;
        for (const auto& [lhs, rhsList] : P) {
            for (const auto& rhs : rhsList) {
                bool ok = true;
                for (const auto& sym : rhs)
                    if (isNonTerminal(sym) && !productive.count(sym)) { ok = false; break; }
                if (ok) {
                    auto& v = newP[lhs];
                    if (std::find(v.begin(), v.end(), rhs) == v.end()) v.push_back(rhs);
                }
            }
        }
        P = newP;
    }

    void convertToCNF() {
        std::map<std::string, std::string> termMap;
        PMap newP;

        auto getTermNT = [&](const std::string& t) -> std::string {
            if (!termMap.count(t)) {
                std::string nt = freshNT("T_");
                termMap[t] = nt;
                newP[nt] = {{t}};
            }
            return termMap[t];
        };

        for (const auto& [lhs, rhsList] : P) {
            for (const auto& rhs : rhsList) {
                if (rhs.size() == 1) {
                    newP[lhs].push_back(rhs);
                } else {
                    RHS replaced;
                    for (const auto& sym : rhs)
                        replaced.push_back(isTerminal(sym) ? getTermNT(sym) : sym);
                    newP[lhs].push_back(replaced);
                }
            }
        }
        P = newP;

        PMap binaryP;
        for (const auto& [lhs, rhsList] : P) {
            for (const auto& rhs : rhsList) {
                if (rhs.size() <= 2) {
                    binaryP[lhs].push_back(rhs);
                } else {
                    std::string cur = lhs;
                    RHS remaining = rhs;
                    while (remaining.size() > 2) {
                        std::string next = freshNT("X");
                        binaryP[cur].push_back({remaining[0], next});
                        remaining.erase(remaining.begin());
                        cur = next;
                    }
                    binaryP[cur].push_back(remaining);
                }
            }
        }
        P = binaryP;
    }

    void toCNF() {
        print("INITIAL GRAMMAR");

        std::cout << "\n Step 1: Eliminate eps-productions \n";
        eliminateEpsilon();
        print("After Step 1");

        std::cout << "\n Step 2: Eliminate unit productions \n";
        eliminateUnitProductions();
        print("After Step 2");

        std::cout << "\n Step 3: Eliminate inaccessible symbols \n";
        eliminateInaccessible();
        print("After Step 3");

        std::cout << "\n Step 4: Eliminate non-productive symbols \n";
        eliminateNonProductive();
        print("After Step 4");

        std::cout << "\n Step 5: Convert to CNF \n";
        convertToCNF();
        print("[OK] FINAL CNF GRAMMAR");
    }
};
