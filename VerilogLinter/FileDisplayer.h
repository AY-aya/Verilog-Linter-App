#ifndef FILEDISPLAYER_H
#define FILEDISPLAYER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStringList>

class FileDisplayer : public QObject
{
    Q_OBJECT
public:
    explicit FileDisplayer(QObject *parent = nullptr);

    QStringList readFileLines(const QString &filePath);

signals:
};

#endif // FILEDISPLAYER_H
