#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <regex>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <cmath>

using namespace std;

// Violation structure to store detected issues
struct Violation
{
    string checkName;
    int lineNumber;
    string impactedVariables;
};

// Structure to store line number and register name
struct InferredLatch {
    int lineNum;
    string regName;
};

//FSM structures
struct Transition {
    string from_state;
    string to_state;
};

struct FSM {
    vector<string> states;            // All detected states
    vector<Transition> transitions;  // Transitions between states
    string initial_state;            // Initial state
};

// Structure to hold analysis results for each case block
struct CaseBlockAnalysis {
    unordered_set<string> covered_cases;  // Detected cases in the block
    unordered_set<string> missing_cases;  // Missing cases for completeness
    unordered_map<string, string> non_parallel_cases;  // Cases that are overlapping or non-parallel
    bool is_full_case;  // True if all possible cases are covered
    bool is_parallel_case;  // True if no overlapping cases exist
};

//global vector for violations
vector<Violation> violations;

class ArithmeticOverflowChecker{

    public:
        // Function to extract the bit-width of an operand
        static int getOperandWidth(const string& operand) {
            regex widthRegex(R"(\[(\d+):\d+\])");
            smatch match;

            if (regex_search(operand, match, widthRegex)) {
                return stoi(match[1].str()) + 1; // Add 1 because bit indices start at 0
            }

            // Default width if no explicit width is provided
            return 1; // Assume 32 bits
        }

        // Function to calculate the maximum possible value for a given bit-width
        static int calculateMaxValue(int bitWidth) {
            return (1 << bitWidth) - 1; // 2^n - 1
        }

        // Function to parse a binary, hexadecimal, or decimal number
        static int parseValue(const string& operand) {
            regex binRegex(R"((\d+)'b([01]+))");
            regex hexRegex(R"((\d+)'h([0-9a-fA-F]+))");
            regex decRegex(R"(\d+)");

            smatch match;

            // Match binary numbers
            if (regex_match(operand, match, binRegex)) {
                return stoi(match[2].str(), nullptr, 2); // Convert binary to decimal
            }

            // Match hexadecimal numbers
            if (regex_match(operand, match, hexRegex)) {
                return stoi(match[2].str(), nullptr, 16); // Convert hex to decimal
            }

            // Match decimal numbers
            if (regex_match(operand, match, decRegex)) {
                return stoi(operand); // Already in decimal
            }

            return 0; // Default to 0 if invalid
        }

        // Function to evaluate the RHS value
        static int evaluateRHS(const string& regName, int regValue, const string& rhs) {
            regex arithmeticOp(R"(([\w'bh]+)\s([\+\-\*\/])\s([\w'bh]+))");
            smatch match;
            //cout<< "1111"<<endl;
            if (regex_match(rhs, match, arithmeticOp)) {
                //cout<<"222"<<endl;
                string operand1 = match[1].str();
                string operation = match[2].str();
                string operand2 = match[3].str();

                // Parse the second operand
                int value2 = parseValue(operand2);
                int value1 = parseValue(operand1);
                // Handle division by zero
                if (operation == "/" && value2 == 0) {
                    cerr << "Error: Division by zero in operation '" << rhs << "'." << endl;
                    return -1; // Special return value to indicate an error
                }

                // Perform the operation
                if (operation == "+") return value1 + value2;
                if (operation == "-") return value1 - value2;
                if (operation == "*") return value1 * value2;
                if (operation == "/") return value1 / value2;
                
                return value1;
            }

            return 0; // Invalid format
        }

        // Function to check for arithmetic overflow
        static void checkArithmeticOverflow(const string& reg, const string& rhs, int lineNum) {
            int regWidth = getOperandWidth(reg); // Extract bit-width
            int regMaxValue = calculateMaxValue(regWidth); // Calculate maximum value
            int regValue = regMaxValue; // Assume the register starts at its maximum value

            int rhsValue = evaluateRHS(reg, regValue, rhs); // Calculate RHS value

            // If division by zero was detected, return early
            if (rhsValue == -1) {
                cout << "---------------------------------------------------" << endl;
                return;
            }


            if (rhsValue > regMaxValue) {
                cout << "Overflow detected!" << endl;
                cout << "Register: " << reg << " | Width: " << regWidth << " bits" << endl;
                cout << "Maximum LHS value: " << regMaxValue << endl;
                cout << "RHS operation: " << rhs << endl;
                cout << "Calculated RHS value: " << rhsValue << endl;
                cout << "---------------------------------------------------" << endl;
                violations.push_back({"Arithmetic Overflow", lineNum ,"Impacted Signal: "+reg});
            }
                
        }

};

class ReportGenerator{
    public:
        // Generate a report file name
        static string generate_filename()
        {
            return "verilinter_report.txt"; // Static filename for simplicity
        }

        // Generate the report of detected violations
        static void generateReport(vector<Violation> violations)
        {
            string reportFileName = generate_filename();
            ofstream reportFile(reportFileName);

            if (!reportFile.is_open())
            {
                cerr << "Error: Unable to open report file " << reportFileName << endl;
                return;
            }

            // Debug: Check if violations are being detected
            if (violations.empty())
            {
                cout << "No violations detected." << endl;
                reportFile << "No violations detected." << endl;
                reportFile.close();
                return;
            }

            //For Organisation: group each type of check together
            //Arithmetic Overflow
            for (const auto &violation : violations)
            {
                if(violation.checkName != "Arithmetic Overflow")
                    continue;
                reportFile << "Check: " << violation.checkName << endl;
                reportFile << "Line: " << violation.lineNumber << endl;
                reportFile << "Impact: " << violation.impactedVariables << endl;
                reportFile << "------------------------------------" << endl;
            }
            //Uninitialized Register
            for (const auto &violation : violations)
            {
                if(violation.checkName != "Uninitialized Register")
                    continue;
                reportFile << "Check: " << violation.checkName << endl;
                reportFile << "Line: " << violation.lineNumber << endl;
                reportFile << "Impact: " << violation.impactedVariables << endl;
                reportFile << "------------------------------------" << endl;
            }
            //Multi-driven Bus Conflict
            for (const auto &violation : violations)
            {
                if(violation.checkName != "Multi-driven Bus Conflict")
                    continue;
                reportFile << "Check: " << violation.checkName << endl;
                if(violation.lineNumber == -1)
                    reportFile << "Line: " << "--" << endl;
                else
                    reportFile << "Line: " << violation.lineNumber << endl;
                reportFile << "Impact: " << violation.impactedVariables << endl;
                reportFile << "------------------------------------" << endl;
            }
            //Unreachable Block
            for (const auto &violation : violations)
            {
                if(violation.checkName != "Unreachable Block")
                    continue;
                reportFile << "Check: " << violation.checkName << endl;
                reportFile << "Line: " << violation.lineNumber << endl;
                reportFile << "Impact: " << violation.impactedVariables << endl;
                reportFile << "------------------------------------" << endl;
            }
            //Inferred Latch
            for (const auto &violation : violations)
            {
                if(violation.checkName != "Inferred Latch")
                    continue;
                reportFile << "Check: " << violation.checkName << endl;
                reportFile << "Line: " << violation.lineNumber << endl;
                reportFile << "Impact: " << violation.impactedVariables << endl;
                reportFile << "------------------------------------" << endl;
            }
            //Unreachable FSM State
            for (const auto &violation : violations)
            {
                if(violation.checkName != "Unreachable FSM State")
                    continue;
                reportFile << "Check: " << violation.checkName << endl;
                //reportFile << "Line: " << violation.lineNumber << endl;
                reportFile << "Impact: " << violation.impactedVariables << endl;
                reportFile << "------------------------------------" << endl;
            }
            //Non Full/Parallel Case
            for (const auto &violation : violations)
            {
                if(violation.checkName != "Non Full/Parallel Case")
                    continue;
                reportFile << "Check: " << violation.checkName << endl;
                //reportFile << "Line: " << violation.lineNumber << endl;
                reportFile << "Impact: " << violation.impactedVariables << endl;
                reportFile << "------------------------------------" << endl;
            }
            

            cout << "Report generated successfully: " << reportFileName << endl;
            

            reportFile.close();
        }

};

class FSMChecker{
    public:
        // --- FSM Checker ---
        static void findUnreachableStates(const FSM& fsm) {
            // Create a graph representation
            unordered_map<string, vector<string>> graph;
            for (const auto& transition : fsm.transitions) {
                graph[transition.from_state].push_back(transition.to_state);
            }

            // Perform BFS to find reachable states
            unordered_set<string> reachable_states;
            queue<string> q;

            // Start BFS from the initial state
            q.push(fsm.initial_state);
            reachable_states.insert(fsm.initial_state);

            while (!q.empty()) {
                string current = q.front();
                q.pop();

                for (const auto& neighbor : graph[current]) {
                    if (reachable_states.find(neighbor) == reachable_states.end()) {
                        reachable_states.insert(neighbor);
                        q.push(neighbor);
                    }
                }
            }

            // Find unreachable states
            unordered_set<string> unreachable_states;
            for (const auto& state : fsm.states) {
                if (reachable_states.find(state) == reachable_states.end()) {
                    unreachable_states.insert(state);
                }
            }

            cout << "\nUnreachable States:\n";
            for (const auto& state : unreachable_states) {
                cout << state << endl;
                violations.push_back({"Unreachable FSM State", -1, "unreachable state: "+state});
            }
        }


};

class CaseChecker{
    public: 
        // Helper function to generate all possible cases for a given bit-width
        static unordered_set<string> generateAllBinaryCases(int bit_width) {
            unordered_set<string> cases;
            int total_cases = pow(2, bit_width); // 2^bit_width

            for (int i = 0; i < total_cases; ++i) {
                // Generate binary string for the current case
                string binary = to_string(bit_width) + "'b" + bitset<32>(i).to_string().substr(32 - bit_width);
                cases.insert(binary);
            }
            return cases;
        }

                
        // Function to parse symbolic constants
        static unordered_map<string, string> parseSymbolicConstants(const string& code) {
            unordered_map<string, string> symbol_to_binary;
            regex param_regex(R"((parameter|localparam)\s+(\w+)\s*=\s*(\d+'b[01]+);)");
            smatch match;
            string::const_iterator search_start(code.cbegin());

            while (regex_search(search_start, code.cend(), match, param_regex)) {
                string symbol = match[2];
                string binary_value = match[3];
                symbol_to_binary[symbol] = binary_value;
                search_start = match.suffix().first;
            }

            return symbol_to_binary;
        }

        // Function to replace symbolic cases with binary equivalents
        static string replaceSymbolicCases(const string& case_body, const unordered_map<string, string>& symbol_to_binary) {
            string replaced_body = case_body;
            for (const auto& [symbol, binary] : symbol_to_binary) {
                regex symbol_regex("\\b" + symbol + "\\b");
                replaced_body = regex_replace(replaced_body, symbol_regex, binary);
            }
            return replaced_body;
        }

        // Case block analysis function
        static CaseBlockAnalysis analyzeCaseBlock(const string& case_body) {
            CaseBlockAnalysis block_analysis;
            block_analysis.is_parallel_case = true;
            block_analysis.is_full_case = true;

            regex case_item_regex(R"((\d+'b[01]+|default)\s*:)");

            smatch case_item_match;
            unordered_map<string, int> case_counts;
            bool has_default = false;

            string::const_iterator case_search_start(case_body.cbegin());
            while (regex_search(case_search_start, case_body.cend(), case_item_match, case_item_regex)) {
                string detected_case = case_item_match[1];
                case_search_start = case_item_match.suffix().first;

                if (detected_case == "default") {
                    has_default = true;
                } else {
                    case_counts[detected_case]++;
                    if (case_counts[detected_case] > 1) {
                        block_analysis.is_parallel_case = false;
                        block_analysis.non_parallel_cases[detected_case] = "Duplicate";
                    }
                    block_analysis.covered_cases.insert(detected_case);
                }
            }

            int bit_width = 0;
            regex bit_width_regex(R"(\d+'b[01]+)");
            smatch bit_width_match;
            string::const_iterator search_start(case_body.cbegin());
            while (regex_search(search_start, case_body.cend(), bit_width_match, bit_width_regex)) {
                int current_bit_width = stoi(bit_width_match[0].str().substr(0, bit_width_match[0].str().find("'")));
                bit_width = max(bit_width, current_bit_width);
                search_start = bit_width_match.suffix().first;
            }

            if (!block_analysis.covered_cases.empty()) {
                unordered_set<string> all_cases = generateAllBinaryCases(bit_width);
                for (const auto& expected : all_cases) {
                    if (block_analysis.covered_cases.find(expected) == block_analysis.covered_cases.end() && !has_default) {
                        block_analysis.is_full_case = false;
                        block_analysis.missing_cases.insert(expected);
                    }
                }
            } else {
                block_analysis.is_full_case = false;
            }

            return block_analysis;
        }

        

};

class BlockChecker{
    public:
        // Function to check if a value is a binary value (e.g., 'b1010')
        static bool isBinaryValue(const string &value) {
            regex binaryRegex(R"(\d+'b[01]+)"); // Matches binary values like 2'b1010
            return regex_match(value, binaryRegex);
        }

        // Extract assigned values and their line numbers
        static map<string, int> extractAssignedValuesWithLines(const string &code, const string &variable) {
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

        // Extract case states and their line numbers
        static map<string, int> extractCaseStatesWithLines(const string &code, const string &targetVariable) {
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

        // Check FSM violations for binary values
        static void checkFSMViolations(const map<string, int> &assignedValues, const map<string, int> &caseStates) {
            cout << "unreachable statement Violations:\n";
            for (map<string, int>::const_iterator it = caseStates.begin(); it != caseStates.end(); ++it) {
                const string &state = it->first;
                int line = it->second;

                // Ignore default case
                if (state == "default") continue;

                // Check if state is not in assigned values
                if (assignedValues.find(state) == assignedValues.end()) {
                    cout << "  Unreachable state: " << state << " (on line " << line << ")\n";
                    violations.push_back({"Unreachable Block", line, "State: "+state});
                }
            }
        }

        // Extract all case variables
        static vector<string> extractAllCaseVariables(const string &verilogCode) {
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

};

class LatchChecker{
    public:
        // Function to extract registers assigned in lhs of assignments
        static vector<string> extractRegistersFromLHS(const string& line) {
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

};

class UninitialisedChecker{
    public:
        static void detectUninitializedRegisters(const string &code)
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
                    string signals = match[3]; // List of signal names

                    // Split signals by commas and trim whitespace
                    stringstream signalStream(signals);
                    string reg;
                    while (getline(signalStream, reg, ','))
                    {
                        reg = regex_replace(reg, regex("^\\s+|\\s+$"), ""); // Trim spaces
                        registers.insert(reg);                              // Register declaration found, add to set
                        cout << "Found reg: " << reg << endl;
                    }
                    cout << "----------------------------------------------------\n";
                }

                // Detect initialization (e.g., a = b + 1 or e <= a + 1)
                while (regex_search(line, match, initRegex))
                {
                    string targetReg = match[1];   // Target register being initialized
                    initialized.insert(targetReg); // Mark target register as initialized
                    cout << "target reg: " << targetReg << endl;

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
                                cout << "Register '" << rhsReg << "' used before initialization.\n";
                                uninitializedUsed.insert(rhsReg);
                                flaggedRegisters.insert(rhsReg);
                                foundUninitialized = true;
                                violations.push_back({"Uninitialized Register", lineNum, "Reg: "+rhsReg+" used before initialization"});
                            }
                            usageCount[rhsReg]++;
                        }
                        rhs = rhsMatch.suffix();
                    }
                    cout << "----------------------------------------------------\n";
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
                                cout << "Register '" << condReg << "' used in `if` condition before initialization.\n";
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
                                cout << "Register '" << condReg << "' used in `while` condition before initialization.\n";
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
                cout << "\nUninitialized Register Usage Detected:\n";
                for (auto entry : usageCount)
                {
                    string reg = entry.first;
                    int count = entry.second;

                    if (count >= 0 || uninitializedUsed.count(reg))
                    {
                        cout << "Register '" << reg << "' used " << count << " times before initialization.\n";
                        //violations.push_back("Uninitialized Register",)
                    }
                }
            }
            else
            {
                cout << "No uninitialized register usage detected.\n";
            }
        }

};

class ParserEngine{

    public:
        // Tracks global and conditional signal drivers
        struct SignalDrivers {
            unordered_set<int> global_drivers; // Line numbers for global assignments (assign, always without conditions)
            vector<unordered_map<string, int>> conditional_drivers; // Nested conditional branches
        };
        
        //returns the whole code as a string
        static string readFile(const string& file_path) {
            ifstream file(file_path);
            if (!file.is_open()) {
                throw runtime_error("Failed to open file: " + file_path);
            }

            stringstream buffer;
            buffer << file.rdbuf();
            return buffer.str();
        }

        //calls each parser function
        /*
            * fsm parser takes string for the whole code
            * overflow, bus conflict parsers are called line by line
        */
        static void parseFile(const string& filePath, bool arithmetic_c, bool unReachableBlck_c, bool unReachableFSM_c,
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
            cout<<"--------------------------------------------------------------------------\n";
            if(fullParCase_c)
                parseCases(code);
            cout<<"--------------------------------------------------------------------------\n";
            if(unReachableBlck_c)
                parseBlocks(code);
            cout<<"--------------------------------------------------------------------------\n";
            if(latches_c)
                parseInferredLatches(code);
            cout<<"--------------------------------------------------------------------------\n";
            if(unInitReg_c)
                UninitialisedChecker::detectUninitializedRegisters(code);
            cout<<"--------------------------------------------------------------------------\n";
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
                            cerr << "Multi-driven conflict detected for signal '" << signal << "' driven in line: " << line << "\n";
                            violations.push_back({"Multi-driven Bus Conflict", -1, "Conflict in Signal: "+signal});
                        }
                        i++;
                    }
                }
            }
            
            verilogFile.close();
        }

        //Arithmetic overflow parser
        static void parseOverflow(string line, int lineNum){
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
                    cout << "Found register: " << regName << " | Width: " << bitWidth << " bits" << endl;
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

        //Multi-driven bus conflict parser & checker
        static unordered_map<string, SignalDrivers> parseMultiDrivenBus(string line, int lineNum){
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
                string signals = match[3];   // List of signal names

                // Split signals by commas and trim whitespace
                stringstream signalStream(signals);
                string signalName;
                while (getline(signalStream, signalName, ',')) {
                    signalName = regex_replace(signalName, regex("^\\s+|\\s+$"), ""); // Trim spaces
                    if (signalDrivers.find(signalName) == signalDrivers.end()) {
                        signalDrivers[signalName] = SignalDrivers();
                    } else {
                        cerr << "Error: Signal '" << signalName << "' is redeclared on line " << lineNum << endl;
                    }
                }
            }

            // Check for assign statements
            if (regex_search(line, match, assignRegex)) {
                string signalName = match[1];
                if (signalDrivers.find(signalName) == signalDrivers.end()) {
                    cerr << "Error: Undeclared signal '" << signalName << "' assigned on line " << lineNum << endl;
                    return signalDrivers;
                }
                auto& drivers = signalDrivers[signalName];
                if (!drivers.global_drivers.insert(lineNum).second) {
                    cerr << "Conflict detected: Signal '" << signalName
                            << "' is driven multiple times in assign statements (line " << lineNum << ")\n";
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
                    cerr << "Error: Undeclared signal '" << signalName << "' assigned on line " << lineNum << endl;
                    return signalDrivers;
                }
                auto& drivers = signalDrivers[signalName];

                if (!caseStack.empty()) {
                    // Inside a case statement
                    auto& currentBranch = caseStack.top();
                    if (currentBranch.find(signalName) != currentBranch.end()) {
                        cerr << "Multi-driven conflict detected in case branch: Signal '" << signalName
                            << "' is driven multiple times within the same branch on line " << lineNum << "\n";
                        violations.push_back({"Multi-driven Bus Conflict", lineNum, "Conflict in Signal: "+signalName});
                    } else {
                        currentBranch[signalName] = lineNum;
                    }
                } else if (currentConditionalLevel == -1) {
                    // No conditionals: Treat as global driver in always block
                    if (!drivers.global_drivers.insert(lineNum).second) {
                        cerr << "Multi-driven conflict detected: Signal '" << signalName
                            << "' is driven multiple times in always block (line " << lineNum << ")\n";
                        violations.push_back({"Multi-driven Bus Conflict", lineNum, "Conflict in Signal: "+signalName});
                    }
                } else {
                    // Inside a conditional: Track by conditional level
                    while (drivers.conditional_drivers.size() <= currentConditionalLevel) {
                        drivers.conditional_drivers.emplace_back();
                    }
                    auto& currentBranch = drivers.conditional_drivers[currentConditionalLevel];
                    if (currentBranch.find(signalName) != currentBranch.end()) {
                        cerr << "Multi-driven conflict detected in conditional block: Signal '" << signalName
                            << "' is driven multiple times within the same branch on line " << lineNum << "\n";
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

        //Fsm parser
        static void parseFSM(const string& code) {
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
            cout << "Parsed States:\n";
            for (const auto& state : fsm.states) {
                cout << state << endl;
            }
            cout << "\nParsed Transitions:\n";
            for (const auto& t : fsm.transitions) {
                cout << t.from_state << " -> " << t.to_state << endl;
            }
            cout << "\nInitial State: " << fsm.initial_state << "\n";

            //return fsm;
            cout << "Detected States:\n";
            for (const auto& state : fsm.states) {
                cout << state << endl;
            }
            //check unreachable states
            FSMChecker::findUnreachableStates(fsm);
        }

        //Non Full/Parallel Case parser
        static void parseCases(const string& code){
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
                cout << "Case Block #" << case_block_number++ << ":\n";
                cout << "Covered Cases:\n";
                for (const auto& c : block_analysis.covered_cases) {
                    cout << c << endl;
                }

                cout << "\nIs Full Case: " << (block_analysis.is_full_case ? "Yes" : "No") << endl;
                if (!block_analysis.is_full_case) {
                    cout << "Missing Cases:\n";
                    for (const auto& missing : block_analysis.missing_cases) {
                        cout << missing << endl;
                    }
                    violations.push_back({"Non Full/Parallel Case", -1,"Non full Case Block No.: "+blockNum});
                }

                cout << "\nIs Parallel Case: " << (block_analysis.is_parallel_case ? "Yes" : "No") << endl;
                if (!block_analysis.is_parallel_case) {
                    cout << "Non-Parallel Cases:\n";
                    for (const auto& non_parallel_case : block_analysis.non_parallel_cases) {
                        cout << non_parallel_case.first << " (" << non_parallel_case.second << ")\n";
                    }
                    violations.push_back({"Non Full/Parallel Case", -1,"Non Parallel Case Block No.: "+blockNum});
                }

                cout << "---------------------------------\n";
            }

        
        }

        //Unreachable Blocks parser
        static void parseBlocks(const string& code){
            // Extract case variables
            vector<string> caseVariables = BlockChecker::extractAllCaseVariables(code);
            for (vector<string>::const_iterator varIt = caseVariables.begin(); varIt != caseVariables.end(); ++varIt) {
                const string &variable = *varIt;
                cout << "Checking case variable: " << variable << "\n";

                // Extract assigned values
                map<string, int> assignedValues = BlockChecker::extractAssignedValuesWithLines(code, variable);
                if (assignedValues.empty()) {
                    // If no binary assignments, skip checks
                    cout << "Skipping further checks for " << variable << " due to no binary assignments.\n";
                    continue;
                }

                // Extract case states
                map<string, int> caseStates = BlockChecker::extractCaseStatesWithLines(code, variable);

                // Perform FSM violation checks
                BlockChecker::checkFSMViolations(assignedValues, caseStates);
            }
        }

        //latches Parser
        static void parseInferredLatches(const string& code) {
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
                    cout<<"line: "<<line<<endl;
                    vector<string> regs = LatchChecker::extractRegistersFromLHS(line);
                    if (!regs.empty()) {
                        for (const auto& reg : regs) {
                            ifRegs.push_back({lineNum, reg});  // Store line number along with reg name
                        }
                    }
                }

                // Record registers in lhs of assignment statements inside 'else' block
                if (inElseBlock) {
                    cout<<"line: "<<line<<endl;
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
                    cout<<"else found\n";
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

};



int main(){
    bool arithmetic = false;
    bool unReachableBlck = false;
    bool unReachableFSM = false;
    bool unInitReg = false;
    bool busReg = false;
    bool fullParCase = false;
    bool latches = false;

    string filePath= "sample_3.v";
    string temp;
    cout<<"Please enter your verilog file path: \n";
    getline(cin, filePath);
    cout<<"For each check type, please enter 'yes' to enable and 'no' to disable \n";
    
    cout<<"Arithmetic Overflow checker: ";
    getline(cin, temp);
    if(temp == "yes")
        arithmetic = true;

    cout<<"unReachable Blocks Checker: ";
    getline(cin, temp);
    if(temp == "yes")
        unReachableBlck = true;

    cout<<"unReachable FSM Checker: ";
    getline(cin, temp);
    if(temp == "yes")
        unReachableFSM = true;

    cout<<"unInitialised Reg Checker: ";
    getline(cin, temp);
    if(temp == "yes")
        unInitReg = true;

    cout<<"Multi-driven bus Checker: ";
    getline(cin, temp);
    if(temp == "yes")
        busReg = true;

    cout<<"Non-full/parallel cases Checker: ";
    getline(cin, temp);
    if(temp == "yes")
        fullParCase = true;

    cout<<"Inferred latches Checker: ";
    getline(cin, temp);
    if(temp == "yes")
        latches = true;

    ParserEngine::parseFile(filePath, arithmetic, unReachableBlck, unReachableFSM, unInitReg, busReg, fullParCase, latches);
    ReportGenerator::generateReport(violations);


}


