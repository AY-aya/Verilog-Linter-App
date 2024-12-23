#ifndef CASECHECKER_H
#define CASECHECKER_H

#include <QObject>
#include "VerilogLintChecker.h"
class CaseChecker : public QObject
{
    Q_OBJECT
public:
    explicit CaseChecker(QObject *parent = nullptr);
    // Helper function to generate all possible cases for a given bit-width
    static unordered_set<string> generateAllBinaryCases(int bit_width);


    // Function to parse symbolic constants
    static unordered_map<string, string> parseSymbolicConstants(const string& code);

    // Function to replace symbolic cases with binary equivalents
    static string replaceSymbolicCases(const string& case_body, const unordered_map<string, string>& symbol_to_binary);

    // Case block analysis function
    static CaseBlockAnalysis analyzeCaseBlock(const string& case_body);



signals:
};

#endif // CASECHECKER_H
