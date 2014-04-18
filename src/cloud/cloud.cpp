#include <common/common.h>
#include <common/config.h>
#include <encloud/Progress>
#include <cloud/cloud.h>

namespace libencloud {

//
// public methods
//

Cloud::Cloud (Config *cfg) 
    : _cfg(cfg)
    , _vpnClient(NULL)
    , _vpnManager(NULL)
{
    LIBENCLOUD_TRACE;

    _vpnClient = new VpnClient(_cfg);
    LIBENCLOUD_ERR_IF (_vpnClient == NULL);

    _vpnManager = new VpnManager(_cfg);
    LIBENCLOUD_ERR_IF (_vpnManager == NULL);

    // state change signals handled locally then routed
    connect(_vpnClient, SIGNAL(stateChanged(VpnClient::State)),
            this, SLOT(_vpnStateChanged(VpnClient::State)));

    // ip assignment and need requests (e.g. auth) routed from manager
    connect(_vpnManager, SIGNAL(ipAssigned(QString)),
            this, SIGNAL(ipAssigned(QString)));

    // local error handling
    connect(_vpnClient, SIGNAL(sigError(VpnClient::Error, QString)), 
            this, SLOT(_vpnClientErr(VpnClient::Error, QString)));
    connect(_vpnManager, SIGNAL(sigError(VpnManager::Error, QString)), 
            this, SLOT(_vpnManagerErr(VpnManager::Error, QString)));

    // authentication forwarding to client, to and from manager
    connect(_vpnManager, SIGNAL(authRequired(Auth::Id)), 
            this, SIGNAL(authRequired(Auth::Id)));
    connect(this, SIGNAL(authSupplied(Auth)), 
            _vpnClient, SLOT(authSupplied(Auth)));
    connect(this, SIGNAL(authSupplied(Auth)), 
            _vpnManager, SLOT(authSupplied(Auth)));

    connect(&_retry, SIGNAL(timeout()), SLOT(_onRetry()));
err:
    return;
}

Cloud::~Cloud() 
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_DELETE(_vpnClient);
    LIBENCLOUD_DELETE(_vpnManager);
}

int Cloud::start() 
{
    LIBENCLOUD_TRACE;

    _vpnClient->start();

	return 0;
}

int Cloud::stop() 
{
    LIBENCLOUD_TRACE;

    _retry.stop();
    _vpnManager->detach();
    _vpnClient->stop();

	return 0;
}

int Cloud::getTotalSteps() const
{
    return VpnClient::StateLast - VpnClient::StateFirst + 1;
}

//
// private slots
//

void Cloud::_vpnStateChanged (VpnClient::State state)
{
    QString stateStr = VpnClient::stateString(state);

//    LIBENCLOUD_DBG("state: " << stateStr);

    switch (state)
    {
        case VpnClient::StateStarted:
            _vpnManager->attach(_vpnClient, LIBENCLOUD_VPN_MGMT_HOST,
                    _cfg->config.vpnMgmtPort);
            break;
        case VpnClient::StateConnecting:
            emit stateChanged(StateConnect);
            break;
        case VpnClient::StateConnected:
            emit stateChanged(StateCloud);
            break;
        default:
            break;
    }

    emit progress(Progress(stateStr, state, 
                VpnClient::StateLast - VpnClient::StateFirst + 1));
}

void Cloud::_vpnClientErr (VpnClient::Error err, const QString &errMsg)
{
    QString msg;

    msg = VpnClient::errorString(err);
    if (errMsg != "")
        msg += "\n\n" + errMsg;

    emit error(Error(msg));
    _restart();
}

// manager socket failures are handled internally and timeout handled here -
// all other errors caught by _vpnClientErr()
void Cloud::_vpnManagerErr (VpnManager::Error err, const QString &errMsg)
{
    QString msg;

    switch (err)
    {
        case VpnManager::ConnTimeout:

            emit error(Error(Error::CodeTimeout));
            _restart();
            break;

        default:
            msg = VpnManager::errorString(err);
            if (errMsg != "")
                msg += "\n\n" + errMsg;

            emit error(Error(msg));
            break;
    }
}

void Cloud::_onRetry ()
{
    _vpnClient->start();
}

//
// private methods
//

void Cloud::_restart ()
{
    // auto-retry only for *ECE
#if !defined(LIBENCLOUD_MODE_QIC)
    _vpnClient->stop();
    _retry.start();
#endif
    // QIC core does full stop()
}

} // namespace libencloud
