#include <encloud/Error>
#include <encloud/Progress>
#include "common.h"
#include "manager.h"

//
// public methods
//

Manager::Manager()
    : _isValid(false)
    , _window(NULL)
    , _server(NULL)
    , _statusApi(NULL)
    , _setupApi(NULL)
    , _cloudApi(NULL)
    , _prevState(libencloud::StateIdle)
{
    SECE_TRACE;

    _server = new libencloud::Server;
    SECE_ERR_IF (_server == NULL);
    SECE_ERR_IF (!_server->isValid());

    _statusApi = new libencloud::StatusApi;
    SECE_ERR_IF (_statusApi == NULL);

    _setupApi = new libencloud::SetupApi;
    SECE_ERR_IF (_setupApi == NULL);

    _cloudApi = new libencloud::CloudApi;
    SECE_ERR_IF (_cloudApi == NULL);

    // <= Setup API
    connect(_statusApi, SIGNAL(apiState(libencloud::State)),
            this, SLOT(_statusApiState(libencloud::State)));

    // <= Cloud API
    connect(this, SIGNAL(actionRequest(QString, libencloud::Params)),
            _cloudApi, SLOT(actionRequest(QString, libencloud::Params)));

    // => GUI in setWindow()

    _isValid = true;

    return;
err:
    SECE_DELETE(_cloudApi);
    SECE_DELETE(_setupApi);
    SECE_DELETE(_statusApi);
    SECE_DELETE(_server);
    return;
}

Manager::~Manager()
{
    SECE_TRACE;

    SECE_DELETE(_cloudApi);
    SECE_DELETE(_setupApi);
    SECE_DELETE(_statusApi);
    SECE_DELETE(_server);
}

int Manager::setWindow (MainWindow *window)
{
    SECE_ERR_IF (window == NULL);

    // MainWindow -> Manager
    connect(window, SIGNAL(toggle()),
            this, SLOT(_toggle()));

    // API -> MainWindow
    connect(_statusApi, SIGNAL(apiState(libencloud::State)),
            window, SLOT(_stateChanged(libencloud::State)));
    connect(_statusApi, SIGNAL(error(libencloud::Error)),
            window, SLOT(_gotError(libencloud::Error)));
    connect(_statusApi, SIGNAL(apiProgress(libencloud::Progress)),
            window, SLOT(_gotProgress(libencloud::Progress)));
    connect(_statusApi, SIGNAL(apiNeed(QString)),
            window, SLOT(_gotNeed(QString)));

    _window = window;

    return 0;
err:
    return ~0;
}

int Manager::run()
{
    SECE_TRACE;

    if (_window)
        _window->show();

    _server->start();

    // no need for SetupApi/portSupply(): on controlled devices we can assume
    // fixed port for Encloud service
    _statusApi->start(SECE_STATUS_PERIOD);

    return 0;
}

//
// private slots
//

void Manager::_toggle()
{
    SECE_TRACE;

    switch (_prevState)
    {
        case libencloud::StateIdle:
            _start();
            break;
        case libencloud::StateError:
        default:
            _stop();
            break;
    }
}

void Manager::_start()
{
    SECE_TRACE;

    emit actionRequest("start", libencloud::Params());
}

void Manager::_stop()
{
    SECE_TRACE;

    emit actionRequest("stop", libencloud::Params());
}

void Manager::_statusApiState (libencloud::State state)
{
    SECE_DBG(libencloud::stateToString(state));

    _prevState = state;
}
