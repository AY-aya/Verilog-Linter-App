#ifndef UNINITIALISEDCHECKER_H
#define UNINITIALISEDCHECKER_H

#include <QObject>
#include "VerilogLintChecker.h"
class UninitialisedChecker : public QObject
{
    Q_OBJECT
public:
    explicit UninitialisedChecker(QObject *parent = nullptr);
    static void detectUninitializedRegisters(const string &code);

signals:
};

#endif // UNINITIALISEDCHECKER_H
