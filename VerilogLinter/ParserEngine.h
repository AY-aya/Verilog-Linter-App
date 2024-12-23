#ifndef PARSERENGINE_H
#define PARSERENGINE_H

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
using namespace std;
class ParserEngine : public QObject
{
    Q_OBJECT
public:
    explicit ParserEngine(QObject *parent = nullptr);
    // Tracks global and conditional signal drivers
    struct SignalDrivers {
        unordered_set<int> global_drivers; // Line numbers for global assignments (assign, always without conditions)
        vector<unordered_map<string, int>> conditional_drivers; // Nested conditional branches
    };

    //returns the whole code as a string
    static string readFile(const string& file_path);

    //calls each parser function
    /*
            * fsm parser takes string for the whole code
            * overflow, bus conflict parsers are called line by line
        */
    static void parseFile(const string& filePath, bool arithmetic_c, bool unReachableBlck_c, bool unReachableFSM_c,
                          bool unInitReg_c, bool busReg_c, bool fullParCase_c, bool latches_c);

    //Arithmetic overflow parser
    static void parseOverflow(string line, int lineNum);

    //Multi-driven bus conflict parser & checker
    static unordered_map<string, SignalDrivers> parseMultiDrivenBus(string line, int lineNum);

    //Fsm parser
    static void parseFSM(const string& code);

    //Non Full/Parallel Case parser
    static void parseCases(const string& code);

    //Unreachable Blocks parser
    static void parseBlocks(const string& code);

    //latches Parser
    static void parseInferredLatches(const string& code);


signals:
};

#endif // PARSERENGINE_H
