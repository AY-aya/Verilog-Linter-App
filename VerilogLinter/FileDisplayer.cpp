#include "FileDisplayer.h"

FileDisplayer::FileDisplayer(QObject *parent)
    : QObject{parent}
{}

QStringList FileDisplayer::readFileLines(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file:" << filePath;
        return {"Error: Unable to open file."};
    }

    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    return lines;
}
