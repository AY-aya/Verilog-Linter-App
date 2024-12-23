#include "VerilogLintChecker.h"
vector<Violation> violations;
VerilogLintChecker::VerilogLintChecker(QObject *parent)
    : QObject{parent}
{}

QStringList VerilogLintChecker::getFileContent(){
    return fileContent;
}

void VerilogLintChecker::parseFile(QString fileName, bool arithmetic_c, bool unReachableBlck_c, bool unReachableFSM_c, bool unInitReg_c, bool busReg_c, bool fullParCase_c, bool latches_c)
{
    arithmetic       = arithmetic_c;
    unReachableBlck  = unReachableBlck_c;
    unReachableFSM   = unReachableFSM_c;
    unInitReg        = unInitReg_c;
    busReg           = busReg_c;
    fullParCase      = fullParCase_c;
    latches          = latches_c;

    string fileN =fileName.toStdString();
    fileN = fileN.substr(8, fileN.length());

    // Find the position of the last slash (forward or backward)
    size_t lastSlashPos = fileN.find_last_of("/\\");

    // Extract the file path (everything before the last slash)
    filePath = fileN.substr(0, lastSlashPos);
    //cout<<filePath<<endl;
    //for a new search clear previous data
    newSearch= true;
    lines.clear();
    violations.clear();
    ParserEngine::parseFile(fileN, arithmetic_c, unReachableBlck_c, unReachableFSM_c, unInitReg_c, busReg_c, fullParCase_c, latches_c);
    //ReportGenerator::generateReport(violations);
    generateReport(violations);

}

string VerilogLintChecker::generate_filename()
{
    return filePath+"/verilinter_report.txt"; // Static filename for simplicity
}

void VerilogLintChecker::generateReport(vector<Violation> violations)
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
        reportFile << "                                    " << endl;
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
        reportFile << "                                    " << endl;
    }


    cout << "Report generated successfully: " << reportFileName << endl;


    reportFile.close();

    fileContent = fileDisplay->readFileLines(QString::fromStdString(reportFileName));
    emit fileReady();
}



