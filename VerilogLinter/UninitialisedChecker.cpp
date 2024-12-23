#include "UninitialisedChecker.h"

UninitialisedChecker::UninitialisedChecker(QObject *parent)
    : QObject{parent}
{}

void UninitialisedChecker::detectUninitializedRegisters(const string &code)
{
    stringstream file(code);
    string line;
    unordered_set<string> registers;         // Holds all registers
    unordered_set<string> initialized;       // Holds initialized registers
    unordered_set<string> uninitializedUsed; // Holds registers used before initialization
    unordered_map<string, int> usageCount;   // Tracks register usage count
    unordered_set<string> flaggedRegisters;  // Registers that have already been flagged for uninitialized usage
    bool foundUninitialized = false;         // Track if any uninitialized register is found

    // Regular expressions for detecting register declarations, initialization, and usage
    regex regDeclRegex(R"((wire|reg)\s*(\[\d+:\d+\])?\s*([\w, ]+)\s*;)"); // Detect register declarations
    regex initRegex(R"(\b(\w+)\s*(<=|\=)\s*([^;]+);)");                   // Detect initialization of registers (both = and <=)
    regex usageRegex(R"(\b(\w+)\b)");                                     // Detect register usage
    regex ifConditionRegex(R"(if\s*\(([^)]+)\))");                        // Detect `if` condition
    regex whileConditionRegex(R"(while\s*\(([^)]+)\))");                  // Detect `while` loop condition

    // First pass to detect register declarations, initialization, and usage
    int lineNum=0;
    while (getline(file, line))
    {
        lineNum++;
        // Remove comments for cleaner parsing
        size_t commentPos = line.find("//");
        if (commentPos != string::npos)
        {
            line = line.substr(0, commentPos);
        }

        smatch match;

        // Detect register declarations (e.g., reg a, b, c;)
        if (regex_search(line, match, regDeclRegex))
        {
            string type = match[1];    // wire or reg
            string vector = match[2];  // Optional [N:M]
            string signalss = match[3]; // List of signal names

            // Split signals by commas and trim whitespace
            stringstream signalStream(signalss);
            string reg;
            while (getline(signalStream, reg, ','))
            {
                reg = regex_replace(reg, regex("^\\s+|\\s+$"), ""); // Trim spaces
                registers.insert(reg);                              // Register declaration found, add to set
                //cout << "Found reg: " << reg << endl;
            }
            //cout << "----------------------------------------------------\n";
        }

        // Detect initialization (e.g., a = b + 1 or e <= a + 1)
        while (regex_search(line, match, initRegex))
        {
            string targetReg = match[1];   // Target register being initialized
            initialized.insert(targetReg); // Mark target register as initialized
            //cout << "target reg: " << targetReg << endl;

            // Check RHS for uninitialized usage
            string rhs = match[3];
            smatch rhsMatch;
            while (regex_search(rhs, rhsMatch, usageRegex))
            {
                string rhsReg = rhsMatch[1];
                if (registers.count(rhsReg) && !initialized.count(rhsReg))
                {
                    if (flaggedRegisters.count(rhsReg) == 0)
                    {
                        //cout << "Register '" << rhsReg << "' used before initialization.\n";
                        uninitializedUsed.insert(rhsReg);
                        flaggedRegisters.insert(rhsReg);
                        foundUninitialized = true;
                        violations.push_back({"Uninitialized Register", lineNum, "Reg: "+rhsReg+" used before initialization"});
                    }
                    usageCount[rhsReg]++;
                }
                rhs = rhsMatch.suffix();
            }
            //cout << "----------------------------------------------------\n";
            line = match.suffix();
        }

        // Detect `if` conditions and flag uninitialized usage
        while (regex_search(line, match, ifConditionRegex))
        {
            string condition = match[1]; // Extract condition inside `if`
            smatch condMatch;
            while (regex_search(condition, condMatch, usageRegex))
            {
                string condReg = condMatch[1];
                if (registers.count(condReg) && !initialized.count(condReg))
                {
                    if (flaggedRegisters.count(condReg) == 0)
                    {
                        //cout << "Register '" << condReg << "' used in `if` condition before initialization.\n";
                        uninitializedUsed.insert(condReg);
                        flaggedRegisters.insert(condReg);
                        foundUninitialized = true;
                        violations.push_back({"Uninitialized Register", lineNum, "Reg: "+condReg+" used before initialization"});
                    }
                    usageCount[condReg]++;
                }
                condition = condMatch.suffix();
            }
            line = match.suffix();
        }

        // Detect `while` conditions and flag uninitialized usage
        while (regex_search(line, match, whileConditionRegex))
        {
            string condition = match[1]; // Extract condition inside `while`
            smatch condMatch;
            while (regex_search(condition, condMatch, usageRegex))
            {
                string condReg = condMatch[1];
                if (registers.count(condReg) && !initialized.count(condReg))
                {
                    if (flaggedRegisters.count(condReg) == 0)
                    {
                        //cout << "Register '" << condReg << "' used in `while` condition before initialization.\n";
                        uninitializedUsed.insert(condReg);
                        flaggedRegisters.insert(condReg);
                        foundUninitialized = true;
                        violations.push_back({"Uninitialized Register", lineNum, "Reg: "+condReg+" used before initialization"});
                    }
                    usageCount[condReg]++;
                }
                condition = condMatch.suffix();
            }
            line = match.suffix();
        }
    }

    // If we found any uninitialized register usage, print them
    if (foundUninitialized)
    {
        //cout << "\nUninitialized Register Usage Detected:\n";
        for (auto entry : usageCount)
        {
            string reg = entry.first;
            int count = entry.second;

            if (count >= 0 || uninitializedUsed.count(reg))
            {
                //cout << "Register '" << reg << "' used " << count << " times before initialization.\n";
                //violations.push_back("Uninitialized Register",)
            }
        }
    }
    else
    {
        //cout << "No uninitialized register usage detected.\n";
    }
}
