#ifndef VERILOGLINTCHECKER_H
#define VERILOGLINTCHECKER_H

#include <QObject>
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
#include "FileDisplayer.h"
#include "ParserEngine.h"
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
extern vector<Violation> violations;


class VerilogLintChecker : public QObject
{
    Q_OBJECT
public:
    explicit VerilogLintChecker(QObject *parent = nullptr);

public slots:

    QStringList getFileContent();
    // Parse the input Verilog file
    void parseFile(QString fileName, bool arithmetic_c, bool unReachableBlck_c, bool unReachableFSM_c,
                   bool unInitReg_c, bool busReg_c, bool fullParCase_c, bool latches_c);

    // Generate a report file name
    string generate_filename();

    // Generate the report of detected violations
    void generateReport(vector<Violation> violations);

signals:
    void fileReady();
private:
    vector<string> lines;         // Store lines of the Verilog file
    //vector<Violation> violations; // Detected violations
    bool arithmetic;
    bool unReachableBlck;
    bool unReachableFSM;
    bool unInitReg;
    bool busReg;
    bool fullParCase;
    bool latches;
    FileDisplayer *fileDisplay;
    QStringList fileContent;
    string filePath;
    bool newSearch;



signals:

};

#endif // VERILOGLINTCHECKER_H
