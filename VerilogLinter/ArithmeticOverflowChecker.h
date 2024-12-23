#ifndef ARITHMETICOVERFLOWCHECKER_H
#define ARITHMETICOVERFLOWCHECKER_H

#include <QObject>
#include "VerilogLintChecker.h"
class ArithmeticOverflowChecker : public QObject
{
    Q_OBJECT
public:
    explicit ArithmeticOverflowChecker(QObject *parent = nullptr);
    // Function to extract the bit-width of an operand
    static int getOperandWidth(const string& operand);

    // Function to calculate the maximum possible value for a given bit-width
    static int calculateMaxValue(int bitWidth);

    // Function to parse a binary, hexadecimal, or decimal number
    static int parseValue(const string& operand);

    // Function to evaluate the RHS value
    static int evaluateRHS(const string& regName, int regValue, const string& rhs);

    // Function to check for arithmetic overflow
    static void checkArithmeticOverflow(const string& reg, const string& rhs, int lineNum);

signals:
};

#endif // ARITHMETICOVERFLOWCHECKER_H
