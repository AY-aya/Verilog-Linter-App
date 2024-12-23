#include "ParserEngine.h"
#include "ArithmeticOverflowChecker.h"
#include "BlockChecker.h"
#include "UninitialisedChecker.h"
#include "LatchChecker.h"
#include "CaseChecker.h"
#include "FSMChecker.h"

ParserEngine::ParserEngine(QObject *parent)
    : QObject{parent}
{}

string ParserEngine::readFile(const string &file_path) {
    ifstream file(file_path);
    if (!file.is_open()) {
        throw runtime_error("Failed to open file: " + file_path);
    }

    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void ParserEngine::parseFile(const string &filePath, bool arithmetic_c, bool unReachableBlck_c, bool unReachableFSM_c,
                             bool unInitReg_c, bool busReg_c, bool fullParCase_c, bool latches_c){
    // Open the Verilog file
    ifstream verilogFile(filePath);
    if (!verilogFile.is_open()) {
        cerr << "Error: Could not open file " << filePath << endl;
        return;
    }
    //checks on whole code
    string code= readFile(filePath);
    if(unReachableFSM_c)
        parseFSM(code);
    //cout<<"--------------------------------------------------------------------------\n";
    if(fullParCase_c)
        parseCases(code);
    //cout<<"--------------------------------------------------------------------------\n";
    if(unReachableBlck_c)
        parseBlocks(code);
    //cout<<"--------------------------------------------------------------------------\n";
    if(latches_c)
        parseInferredLatches(code);
    //cout<<"--------------------------------------------------------------------------\n";
    if(unInitReg_c)
        UninitialisedChecker::detectUninitializedRegisters(code);
    //cout<<"--------------------------------------------------------------------------\n";
    //checks by line:
    string line;
    int lineNum= 0;
    unordered_map<string, SignalDrivers> signalDrivers;

    while (getline(verilogFile, line)){
        lineNum++;
        if(arithmetic_c)
            parseOverflow(line, lineNum);
        if(busReg_c)
            signalDrivers = parseMultiDrivenBus(line, lineNum);
    }

    //needed for bus conflict
    for (const auto& [signal, drivers] : signalDrivers) {
        if (drivers.global_drivers.size() + drivers.conditional_drivers.size() > 1) {
            int i=0;
            for (int line : drivers.global_drivers) {
                if(i< drivers.global_drivers.size() -1){
                    //cerr << "Multi-driven conflict detected for signal '" << signal << "' driven in line: " << line << "\n";
                    violations.push_back({"Multi-driven Bus Conflict", -1, "Conflict in Signal: "+signal});
                }
                i++;
            }
        }
    }

    verilogFile.close();
}

void ParserEngine::parseOverflow(string line, int lineNum){
    // Map to store register names and their bit-widths
    static unordered_map<string, int> registerMap;
    regex regRegex(R"(reg\s*\[(\d+):\d+\]\s*([\w\s,]+);)"); // Match "reg [X:Y] reg_name1, reg_name2;"
    regex assignRegex(R"((\w+)\s*=\s*(.+);)");              // Match "reg_name = expression;"
    smatch match;

    // Trim whitespace
    line = regex_replace(line, regex("^\\s+|\\s+$"), "");

    // Check for register declarations
    if (regex_search(line, match, regRegex)) {
        int bitWidth = stoi(match[1].str()) + 1; // Extract bit-width
        string regList = match[2].str();         // Extract the register list

        // Split the register list into individual register names
        regex regNameRegex(R"(\w+)"); // Matches each register name
        sregex_iterator regBegin(regList.begin(), regList.end(), regNameRegex);
        sregex_iterator regEnd;

        for (auto it = regBegin; it != regEnd; ++it) {
            string regName = it->str();
            registerMap[regName] = bitWidth;
            //cout << "Found register: " << regName << " | Width: " << bitWidth << " bits" << endl;
        }
    }

    // Check for assignments or operations
    else if (regex_search(line, match, assignRegex)) {
        string regName = match[1].str();    // Register on the LHS
        string rhs = match[2].str();        // RHS expression

        // Validate if the register is declared
        if (registerMap.find(regName) != registerMap.end()) {
            // Check for arithmetic overflow
            string regFullName = regName + "[" + to_string(registerMap[regName] - 1) + ":0]";
            ArithmeticOverflowChecker::checkArithmeticOverflow(regFullName, rhs, lineNum);
        } else {
            //cerr << "Warning: Undeclared register used: " << regName << endl;
        }
    }
}

unordered_map<string, ParserEngine::SignalDrivers> ParserEngine::parseMultiDrivenBus(string line, int lineNum){
    static unordered_map<string, SignalDrivers> signalDrivers; // Signal to driver information
    static bool inAlwaysBlock = false;
    static int currentConditionalLevel = -1; // Current level of conditional nesting
    static stack<unordered_map<string, int>> caseStack; // Stack to track case branches

    static regex declarationRegex(R"((wire|reg)\s*(\[\d+:\d+\])?\s*([\w, ]+)\s*;)");
    static regex assignRegex(R"(assign\s+(\w+)\s*=)");
    static regex alwaysBeginRegex(R"(always\s*@\([^)]+\)\s*begin)");
    static regex signalAssignRegex(R"((\w+)\s*(<=|=))");
    static regex conditionalRegex(R"(if\s*\(|else)");
    static regex caseRegex(R"(case\s*\()");
    static regex endRegex(R"(endcase|end)");
    static regex caseBranchRegex(R"(^\s*(\d+|\d'[bdh]\d+|default):)");

    smatch match;

    // Check for wire or reg declarations
    if (regex_search(line, match, declarationRegex)) {
        string type = match[1];       // wire or reg
        string vector = match[2];    // Optional [N:M]
        string signalss = match[3];   // List of signal names

        // Split signals by commas and trim whitespace
        stringstream signalStream(signalss);
        string signalName;
        while (getline(signalStream, signalName, ',')) {
            signalName = regex_replace(signalName, regex("^\\s+|\\s+$"), ""); // Trim spaces
            if (signalDrivers.find(signalName) == signalDrivers.end()) {
                signalDrivers[signalName] = SignalDrivers();
            } else {
                //cerr << "Error: Signal '" << signalName << "' is redeclared on line " << lineNum << endl;
            }
        }
    }

    // Check for assign statements
    if (regex_search(line, match, assignRegex)) {
        string signalName = match[1];
        if (signalDrivers.find(signalName) == signalDrivers.end()) {
            //cerr << "Error: Undeclared signal '" << signalName << "' assigned on line " << lineNum << endl;
            return signalDrivers;
        }
        auto& drivers = signalDrivers[signalName];
        if (!drivers.global_drivers.insert(lineNum).second) {
            //cerr << "Conflict detected: Signal '" << signalName
            //     << "' is driven multiple times in assign statements (line " << lineNum << ")\n";
            violations.push_back({"Multi-driven Bus Conflict", lineNum, "Conflict in Signal: "+signalName});
        }
    }

    // Check for the start of an always block
    if (regex_search(line, alwaysBeginRegex)) {
        inAlwaysBlock = true;
        currentConditionalLevel = -1;
    }

    // Check for the start of a case statement
    if (inAlwaysBlock && regex_search(line, caseRegex)) {
        caseStack.push({});
    }

    // Detect case branch (e.g., 2'd0:, 3'b101:, default:)
    if (!caseStack.empty() && regex_search(line, match, caseBranchRegex)) {
        caseStack.top().clear(); // Clear current branch for new assignments
    }

    // Check for the start of a conditional block
    if (inAlwaysBlock && regex_search(line, match, conditionalRegex) && caseStack.empty()) {
        if (match[0] == "else") {
            currentConditionalLevel += 2;
        } else {
            currentConditionalLevel++;
        }
    }

    // Inside an always block: Check for signal assignments
    if (inAlwaysBlock && regex_search(line, match, signalAssignRegex)) {
        string signalName = match[1];
        if (signalDrivers.find(signalName) == signalDrivers.end()) {
            //cerr << "Error: Undeclared signal '" << signalName << "' assigned on line " << lineNum << endl;
            return signalDrivers;
        }
        auto& drivers = signalDrivers[signalName];

        if (!caseStack.empty()) {
            // Inside a case statement
            auto& currentBranch = caseStack.top();
            if (currentBranch.find(signalName) != currentBranch.end()) {
                //cerr << "Multi-driven conflict detected in case branch: Signal '" << signalName
                //     << "' is driven multiple times within the same branch on line " << lineNum << "\n";
                violations.push_back({"Multi-driven Bus Conflict", lineNum, "Conflict in Signal: "+signalName});
            } else {
                currentBranch[signalName] = lineNum;
            }
        } else if (currentConditionalLevel == -1) {
            // No conditionals: Treat as global driver in always block
            if (!drivers.global_drivers.insert(lineNum).second) {
                //cerr << "Multi-driven conflict detected: Signal '" << signalName
                //     << "' is driven multiple times in always block (line " << lineNum << ")\n";
                violations.push_back({"Multi-driven Bus Conflict", lineNum, "Conflict in Signal: "+signalName});
            }
        } else {
            // Inside a conditional: Track by conditional level
            while (drivers.conditional_drivers.size() <= currentConditionalLevel) {
                drivers.conditional_drivers.emplace_back();
            }
            auto& currentBranch = drivers.conditional_drivers[currentConditionalLevel];
            if (currentBranch.find(signalName) != currentBranch.end()) {
                //cerr << "Multi-driven conflict detected in conditional block: Signal '" << signalName
                //     << "' is driven multiple times within the same branch on line " << lineNum << "\n";
                violations.push_back({"Multi-driven Bus Conflict", lineNum, "Conflict in Signal: "+signalName});
            } else {
                currentBranch[signalName] = lineNum;
            }
        }
    }

    // Check for the end of a block
    if (regex_search(line, match, endRegex)) {
        if (match[0] == "endcase" && !caseStack.empty()) {
            caseStack.pop(); // Exit case block
            //cout<<"ooout"<<endl;
        }
        else if (currentConditionalLevel > -1) {
            currentConditionalLevel--;
        }
        else if (inAlwaysBlock) {
            inAlwaysBlock = false; // Exit always block
            //cout<<"out: "<<match[0]<<" "<<lineNumber<<endl;
        }
    }

    return signalDrivers;
}

void ParserEngine::parseFSM(const string &code) {
    FSM fsm;

    // Updated regex patterns
    regex state_regex(R"(\b([a-zA-Z_][a-zA-Z0-9_]*|'?(\d+)'b[01]+)\s*:)"); // Detect states in case block
    regex transition_regex(R"((\w+|(\d+'b[01]+)):\s*ns\s*=\s*(\w+|(\d+'b[01]+));)"); // Detect transitions
    regex initial_state_regex(R"(if\s*\(reset\)\s*cs\s*=\s*(\w+|(\d+'b[01]+));)"); // Detect initial state

    smatch match;

    // Detect all states from the `case` block
    string::const_iterator search_start(code.cbegin());
    unordered_set<string> unique_states;
    while (regex_search(search_start, code.cend(), match, state_regex)) {
        unique_states.insert(match[1]); // Add state name or binary value
        search_start = match.suffix().first;
    }

    fsm.states.assign(unique_states.begin(), unique_states.end()); // Store unique states

    // Extract transitions
    search_start = code.cbegin();
    while (regex_search(search_start, code.cend(), match, transition_regex)) {
        fsm.transitions.push_back({ match[1], match[3] }); // From state to state
        search_start = match.suffix().first;
    }

    // Detect initial state
    if (regex_search(code, match, initial_state_regex)) {
        fsm.initial_state = match[1];
    }

    // Debugging: Output parsed results
    //cout << "Parsed States:\n";
    for (const auto& state : fsm.states) {
       // cout << state << endl;
    }
    //cout << "\nParsed Transitions:\n";
    for (const auto& t : fsm.transitions) {
        //cout << t.from_state << " -> " << t.to_state << endl;
    }
    //cout << "\nInitial State: " << fsm.initial_state << "\n";

    //return fsm;
    //cout << "Detected States:\n";
    for (const auto& state : fsm.states) {
        //cout << state << endl;
    }
    //check unreachable states
    FSMChecker::findUnreachableStates(fsm);
}

void ParserEngine::parseCases(const string &code){
    unordered_map<string, string> symbol_to_binary = CaseChecker::parseSymbolicConstants(code);

    regex case_block_regex(R"(case\s*\((\w+)\)([\s\S]*?endcase))");

    smatch case_block_match;
    string::const_iterator search_start(code.cbegin());
    int case_block_number = 1;

    while (regex_search(search_start, code.cend(), case_block_match, case_block_regex)) {
        string case_body = case_block_match[2];
        search_start = case_block_match.suffix().first;

        case_body = CaseChecker::replaceSymbolicCases(case_body, symbol_to_binary);

        CaseBlockAnalysis block_analysis = CaseChecker::analyzeCaseBlock(case_body);
        string blockNum = to_string(case_block_number);
        //cout << "Case Block #" << case_block_number++ << ":\n";
        //cout << "Covered Cases:\n";
        case_block_number++;
        for (const auto& c : block_analysis.covered_cases) {
            //cout << c << endl;
        }

        //cout << "\nIs Full Case: " << (block_analysis.is_full_case ? "Yes" : "No") << endl;
        if (!block_analysis.is_full_case) {
            //cout << "Missing Cases:\n";
            for (const auto& missing : block_analysis.missing_cases) {
                //cout << missing << endl;
            }
            violations.push_back({"Non Full/Parallel Case", -1,"Non full Case Block No.: "+blockNum});
        }

        //cout << "\nIs Parallel Case: " << (block_analysis.is_parallel_case ? "Yes" : "No") << endl;
        if (!block_analysis.is_parallel_case) {
            //cout << "Non-Parallel Cases:\n";
            for (const auto& non_parallel_case : block_analysis.non_parallel_cases) {
                //cout << non_parallel_case.first << " (" << non_parallel_case.second << ")\n";
            }
            violations.push_back({"Non Full/Parallel Case", -1,"Non Parallel Case Block No.: "+blockNum});
        }

       // cout << "---------------------------------\n";
    }


}

void ParserEngine::parseBlocks(const string &code){
    // Extract case variables
    vector<string> caseVariables = BlockChecker::extractAllCaseVariables(code);
    for (vector<string>::const_iterator varIt = caseVariables.begin(); varIt != caseVariables.end(); ++varIt) {
        const string &variable = *varIt;
        //cout << "Checking case variable: " << variable << "\n";

        // Extract assigned values
        map<string, int> assignedValues = BlockChecker::extractAssignedValuesWithLines(code, variable);
        if (assignedValues.empty()) {
            // If no binary assignments, skip checks
            //cout << "Skipping further checks for " << variable << " due to no binary assignments.\n";
            continue;
        }

        // Extract case states
        map<string, int> caseStates = BlockChecker::extractCaseStatesWithLines(code, variable);

        // Perform FSM violation checks
        BlockChecker::checkFSMViolations(assignedValues, caseStates);
    }
}

void ParserEngine::parseInferredLatches(const string &code) {
    vector<InferredLatch> inferredLatches;
    unordered_map<int, vector<string>> lineToRegs;
    unordered_map<int, string> lineToCode;

    stringstream ss(code);
    string line;
    int lineNum = 0;
    bool inIfBlock = false;
    bool inElseBlock = false;
    vector<pair<int, string>> ifRegs;   // Store (lineNum, regName) for if block
    vector<pair<int, string>> elseRegs; // Store (lineNum, regName) for else block
    vector<pair<int, string>> caseRegs;
    bool caseIsComplete = false;
    int casesNum = 0;
    // Read through each line of the code
    while (getline(ss, line)) {
        lineNum++;
        lineToCode[lineNum] = line;

        // Check for 'if' statement
        if (line.find("if") != string::npos) {
            inIfBlock = true;
            inElseBlock = false;
            ifRegs.clear();
        }

        // Record registers in the lhs of assignment statements inside 'if' block
        if (inIfBlock) {
            //cout<<"line: "<<line<<endl;
            vector<string> regs = LatchChecker::extractRegistersFromLHS(line);
            if (!regs.empty()) {
                for (const auto& reg : regs) {
                    ifRegs.push_back({lineNum, reg});  // Store line number along with reg name
                }
            }
        }

        // Record registers in lhs of assignment statements inside 'else' block
        if (inElseBlock) {
            //cout<<"line: "<<line<<endl;
            vector<string> regs = LatchChecker::extractRegistersFromLHS(line);
            if (!regs.empty()) {
                for (const auto& reg : regs) {
                    elseRegs.push_back({lineNum, reg});  // Store line number along with reg name
                }
            }
            if(line.find("end")!= string::npos){
                inElseBlock = false;
            }
        }
        // Check for 'else' statement
        if (line.find("else") != string::npos) {
            //cout<<"else found\n";
            inElseBlock = true;
            inIfBlock = false;
            elseRegs.clear();
        }

        // Check for 'case' statement
        if (line.find(" case") != string::npos) {
            caseRegs.clear();
            caseIsComplete = false;

        }
        if(line.find("default") != string::npos){
            caseIsComplete= true;

        }
        // Record registers in lhs of assignment statements inside 'case' block
        if (line.find("endcase") != string::npos) {
            // Check if all cases are covered
            //cout<<"AAAAAA\n";
            for (const auto& reg : caseRegs) {
                //cout<<"BBBBBB\n";
                int count = 0;
                for (const auto& elem : caseRegs) {
                    if (elem.second == reg.second) {  // Compare the string part of the pair
                        count++;
                    }
                }
                //cout<<"reg: "<<reg.second<<" "<<count<<endl;
                if(count != casesNum || !caseIsComplete){
                    inferredLatches.push_back({reg.first, reg.second});
                    violations.push_back({"Inferred Latch", reg.first, "Impacted Signal: "+reg.second});
                }
            }
            caseRegs.clear();
            casesNum =0;
        }

        // Process other statements and ensure completeness of case
        if (line.find(": ") != string::npos || line.find("default") != string::npos) {
            casesNum++;
            //cout<<"CAse: "<<casesNum<<endl;
            vector<string> regs = LatchChecker::extractRegistersFromLHS(line);
            if (!regs.empty()) {
                for (const auto& reg : regs) {
                    caseRegs.push_back({lineNum, reg});  // Store line number along with reg name
                }
            }
        }



    }

    // After parsing all lines, check for inferred latches in if-else
    if (!ifRegs.empty() && !elseRegs.empty()) {
        // Check if registers in 'if' and 'else' are different
        for (const auto& ifReg : ifRegs) {
            bool found = false;
            //cout<<"ifreg: "<<ifReg.second<<endl;
            for (const auto& elseReg : elseRegs) {
                if (ifReg.second == elseReg.second) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                inferredLatches.push_back({ifReg.first, ifReg.second});  // Add to inferred latches
                violations.push_back({"Inferred Latch", ifReg.first, "Impacted Signal: "+ifReg.second});
            }
        }

        // Check elseRegs for registers not in ifRegs
        for (const auto& elseReg : elseRegs) {
            //cout<<"elseReg: "<<elseReg.second<<endl;
            bool found = false;
            for (const auto& ifReg : ifRegs) {
                if (elseReg.second == ifReg.second) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                inferredLatches.push_back({elseReg.first, elseReg.second});  // Add to inferred latches
                violations.push_back({"Inferred Latch", elseReg.first, "Impacted Signal: "+elseReg.second});
            }
        }
    } else {
        for (const auto& ifReg : ifRegs) {
            //cout<<"ifreg: "<<ifReg.second<<endl;
            inferredLatches.push_back({ifReg.first, ifReg.second});  // All regs in if block are inferred latches if no else
            violations.push_back({"Inferred Latch", ifReg.first, "Impacted Signal: "+ifReg.second});
        }
    }
}
