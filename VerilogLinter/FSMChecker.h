#ifndef FSMCHECKER_H
#define FSMCHECKER_H

#include <QObject>
#include "VerilogLintChecker.h"
class FSMChecker : public QObject
{
    Q_OBJECT
public:
    explicit FSMChecker(QObject *parent = nullptr);
    // --- FSM Checker ---
    static void findUnreachableStates(const FSM& fsm);

signals:
};

#endif // FSMCHECKER_H
