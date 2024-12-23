#include "BlockChecker.h"

BlockChecker::BlockChecker(QObject *parent)
    : QObject{parent}
{}

bool BlockChecker::isBinaryValue(const string &value) {
    regex binaryRegex(R"(\d+'b[01]+)"); // Matches binary values like 2'b1010
    return regex_match(value, binaryRegex);
}

map<string, int> BlockChecker::extractAssignedValuesWithLines(const string &code, const string &variable) {
    map<string, int> values;
    regex assignmentRegex(variable + R"( =\s*(\d+'b[01]+|[a-zA-Z_][a-zA-Z0-9_]*);)"); // Match binary values or identifiers
    smatch match;
    istringstream stream(code);
    string line;
    int lineNumber = 1;

    while (getline(stream, line)) {
        if (regex_search(line, match, assignmentRegex)) {
            string value = match[1];
            // Check if the assignment is binary
            if (isBinaryValue(value)) {
                values[value] = lineNumber;
            }
        }
        lineNumber++;
    }
    return values;
}

map<string, int> BlockChecker::extractCaseStatesWithLines(const string &code, const string &targetVariable) {
    map<string, int> states;
    regex caseStartRegex("\\bcase\\s*\\(\\s*" + targetVariable + "\\s*\\)");
    regex caseStateRegex(R"(\b([a-zA-Z_][a-zA-Z0-9_]*|'?(\d+)'b[01]+|default)\s*:)"); // Includes "default"
    regex endcaseRegex(R"(\bendcase\b)");
    smatch match;
    istringstream stream(code);
    string line;
    int lineNumber = 1;
    bool inTargetCase = false;

    while (getline(stream, line)) {
        if (!inTargetCase) {
            if (regex_search(line, match, caseStartRegex)) {
                inTargetCase = true;
            }
        } else {
            if (regex_search(line, match, caseStateRegex)) {
                string caseState = match.str(1);
                states[caseState] = lineNumber;
            }
            if (regex_search(line, endcaseRegex)) {
                break;
            }
        }
        lineNumber++;
    }
    return states;
}

void BlockChecker::checkFSMViolations(const map<string, int> &assignedValues, const map<string, int> &caseStates) {
    //cout << "unreachable statement Violations:\n";
    for (map<string, int>::const_iterator it = caseStates.begin(); it != caseStates.end(); ++it) {
        const string &state = it->first;
        int line = it->second;

        // Ignore default case
        if (state == "default") continue;

        // Check if state is not in assigned values
        if (assignedValues.find(state) == assignedValues.end()) {
            //cout << "  Unreachable state: " << state << " (on line " << line << ")\n";
            violations.push_back({"Unreachable Block", line, "State: "+state});
        }
    }
}

vector<string> BlockChecker::extractAllCaseVariables(const string &verilogCode) {
    regex caseRegex(R"(\bcase\s*\(\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*\))");
    sregex_iterator currentMatch(verilogCode.begin(), verilogCode.end(), caseRegex);
    sregex_iterator endMatch;

    vector<string> caseVariables;
    while (currentMatch != endMatch) {
        caseVariables.push_back((*currentMatch)[1]);
        ++currentMatch;
    }
    return caseVariables;
}
