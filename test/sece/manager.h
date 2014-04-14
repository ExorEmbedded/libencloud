#ifndef _SECE_MANAGER_H_
#define _SECE_MANAGER_H_

#include <encloud/Server>
#include <encloud/Api/StatusApi>
#include <encloud/Api/SetupApi>
#include <encloud/Api/CloudApi>
#include "mainwindow.h"

class Manager : public QObject
{
    Q_OBJECT

public:
    Manager ();
    ~Manager ();

    int run ();
    int setWindow (MainWindow *window);

signals:
    void actionRequest (const QString &action, const libencloud::Params &params);
    void stateChanged (libencloud::State state);

private slots:
    void _toggle ();
    void _start ();
    void _stop ();
    void _statusApiState (libencloud::State state);

private:
    MainWindow *_window;

    // encloud server and API handlers
    libencloud::Server _server;
    libencloud::StatusApi _statusApi;
    libencloud::SetupApi _setupApi;
    libencloud::CloudApi _cloudApi;
    libencloud::State _prevState;
};

#endif  // _SECE_MANAGER_H_
