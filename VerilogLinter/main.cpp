#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include "VerilogLintChecker.h"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    VerilogLintChecker* linter = new VerilogLintChecker(&app);

    app.setWindowIcon(QIcon(":/VerilogLinter/assets/logo.png"));


    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/VerilogLinter/main.qml"));

    qmlRegisterSingletonInstance("com.company.VerilogLintChecker", 1, 0, "VerilogLintChecker", linter);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
