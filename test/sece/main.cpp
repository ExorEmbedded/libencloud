#include <QApplication>
#include <QDir>
#include "common.h"
#include <encloud/Logger>
#include "mainwindow.h"
#include "manager.h"

int main (int argc, char **argv)
{

    QApplication app(argc, argv, true);

    QDir::setCurrent(app.applicationDirPath());

//#ifdef Q_OS_WINCE
    libencloud::Logger fileLogger;
    QString logFile = "./secelog.txt";
    SECE_RETURN_IF (fileLogger.setPath(logFile),
            EXIT_FAILURE);
    SECE_RETURN_IF (fileLogger.open(),
            EXIT_FAILURE);
//#endif

    SECE_DBG("Starting");

    MainWindow mainWindow;
    Manager manager;

    SECE_ERR_IF (!manager.isValid());
    SECE_ERR_IF (manager.setWindow(&mainWindow));
    SECE_ERR_IF (manager.run());

    SECE_ERR_IF (app.exec());


    return 0;
err:
    SECE_DBG("Failure");
    return EXIT_FAILURE;
}
