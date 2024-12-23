#include "ArithmeticOverflowChecker.h"

ArithmeticOverflowChecker::ArithmeticOverflowChecker(QObject *parent)
    : QObject{parent}
{}

int ArithmeticOverflowChecker::getOperandWidth(const string &operand) {
    regex widthRegex(R"(\[(\d+):\d+\])");
    smatch match;

    if (regex_search(operand, match, widthRegex)) {
        return stoi(match[1].str()) + 1; // Add 1 because bit indices start at 0
    }

    // Default width if no explicit width is provided
    return 1; // Assume 32 bits
}

int ArithmeticOverflowChecker::calculateMaxValue(int bitWidth) {
    return (1 << bitWidth) - 1; // 2^n - 1
}

int ArithmeticOverflowChecker::parseValue(const string &operand) {
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

int ArithmeticOverflowChecker::evaluateRHS(const string &regName, int regValue, const string &rhs) {
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

void ArithmeticOverflowChecker::checkArithmeticOverflow(const string &reg, const string &rhs, int lineNum) {
    int regWidth = getOperandWidth(reg); // Extract bit-width
    int regMaxValue = calculateMaxValue(regWidth); // Calculate maximum value
    int regValue = regMaxValue; // Assume the register starts at its maximum value

    int rhsValue = evaluateRHS(reg, regValue, rhs); // Calculate RHS value

    // If division by zero was detected, return early
    if (rhsValue == -1) {
        //cout << "---------------------------------------------------" << endl;
        return;
    }


    if (rhsValue > regMaxValue) {
        /*cout << "Overflow detected!" << endl;
        cout << "Register: " << reg << " | Width: " << regWidth << " bits" << endl;
        cout << "Maximum LHS value: " << regMaxValue << endl;
        cout << "RHS operation: " << rhs << endl;
        cout << "Calculated RHS value: " << rhsValue << endl;
        cout << "---------------------------------------------------" << endl;
        */
        violations.push_back({"Arithmetic Overflow", lineNum ,"Impacted Signal: "+reg});
    }

}
