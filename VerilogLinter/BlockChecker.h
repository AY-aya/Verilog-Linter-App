#ifndef BLOCKCHECKER_H
#define BLOCKCHECKER_H

#include <QObject>
#include "VerilogLintChecker.h"
class BlockChecker : public QObject
{
    Q_OBJECT
public:
    explicit BlockChecker(QObject *parent = nullptr);
    // Function to check if a value is a binary value (e.g., 'b1010')
    static bool isBinaryValue(const string &value);

    // Extract assigned values and their line numbers
    static map<string, int> extractAssignedValuesWithLines(const string &code, const string &variable);

    // Extract case states and their line numbers
    static map<string, int> extractCaseStatesWithLines(const string &code, const string &targetVariable);

    // Check FSM violations for binary values
    static void checkFSMViolations(const map<string, int> &assignedValues, const map<string, int> &caseStates);

    // Extract all case variables
    static vector<string> extractAllCaseVariables(const string &verilogCode);

signals:
};

#endif // BLOCKCHECKER_H
