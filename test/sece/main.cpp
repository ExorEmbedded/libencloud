#include <QApplication>
#include "common.h"
#include "mainwindow.h"
#include "manager.h"

int main (int argc, char **argv)
{
    SECE_DBG("Starting");

    QApplication app(argc, argv, true);
    MainWindow mainWindow;
    Manager manager;

    SECE_ERR_IF (manager.setWindow(&mainWindow));
    SECE_ERR_IF (manager.run());

    SECE_ERR_IF (app.exec());

    return 0;
err:
    SECE_DBG("Failure");
    return EXIT_FAILURE;
}
