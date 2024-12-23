#include "LatchChecker.h"

LatchChecker::LatchChecker(QObject *parent)
    : QObject{parent}
{}

vector<string> LatchChecker::extractRegistersFromLHS(const string &line) {
    vector<string> regs;
    regex regPattern(R"(\s*(\w+)\s*=\s*)");  // Pattern to match the register in lhs of assignment
    smatch match;

    // Use regex_iterator to find all matches in the line
    auto begin = sregex_iterator(line.begin(), line.end(), regPattern);
    auto end = sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        regs.push_back(it->str(1));  // Capture the register name from the first capture group
    }
    return regs;
}
