#ifndef LATCHCHECKER_H
#define LATCHCHECKER_H

#include <QObject>
#include "VerilogLintChecker.h"
class LatchChecker : public QObject
{
    Q_OBJECT
public:
    explicit LatchChecker(QObject *parent = nullptr);
    // Function to extract registers assigned in lhs of assignments
    static vector<string> extractRegistersFromLHS(const string& line);

signals:
};

#endif // LATCHCHECKER_H
