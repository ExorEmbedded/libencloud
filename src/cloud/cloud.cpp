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
    , _setup(NULL)
    , _isFallback(false)
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
    connect(_vpnManager, SIGNAL(authRequired(libencloud::Auth::Id)), 
            this, SIGNAL(authRequired(libencloud::Auth::Id)));
    connect(this, SIGNAL(authSupplied(libencloud::Auth)), 
            _vpnClient, SLOT(authSupplied(libencloud::Auth)));
    connect(this, SIGNAL(authSupplied(libencloud::Auth)), 
            _vpnManager, SLOT(authSupplied(libencloud::Auth)));

    connect(&_retry, SIGNAL(timeout()), SLOT(_onRetry()));
err:
    return;
}

Cloud::~Cloud ()
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_DELETE(_vpnClient);
    LIBENCLOUD_DELETE(_vpnManager);
}

int Cloud::start (bool fallback) 
{
    LIBENCLOUD_TRACE;

    _isFallback = fallback;

    _vpnClient->start(fallback);

	return 0;
}

int Cloud::stop () 
{
    LIBENCLOUD_TRACE;

    _retry.stop();
    _vpnManager->detach();
    _vpnClient->stop();

	return 0;
}

int Cloud::getTotalSteps () const
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
            emit fallback(_isFallback);
            break;
        default:
            break;
    }

    emit progress(Progress(stateStr, state, 
                VpnClient::StateLast - VpnClient::StateFirst + 1));
}

void Cloud::_vpnClientErr (VpnClient::Error err, const QString &errMsg)
{
    // If a valid fallback configuration has been received from
    // Switchboard and we haven't tried it yet, use it
    if (!_isFallback && _setup && _setup->getFallbackVpnConfig() &&
            _setup->getFallbackVpnConfig()->isValid())
    {
        LIBENCLOUD_DBG("[Cloud] Retrying with fallback configuration");
        _restart(true, true);
    }
    else 
    {
        QString msg;

        msg = VpnClient::errorString(err);
        if (errMsg != "")
            msg += "\n\n" + errMsg;

        emit error(Error(Error::CodeClientFailure, msg));

        _restart(_isFallback);
    }
}

// manager socket failures are handled internally and timeout handled here -
// all other errors caught by _vpnClientErr()
void Cloud::_vpnManagerErr (VpnManager::Error err, const QString &errMsg)
{
    QString msg;

    switch (err)
    {
        case VpnManager::ConnTimeout:

            // If a valid fallback configuration has been received from
            // Switchboard and we haven't tried it yet, use it
            if (!_isFallback && _setup && _setup->getFallbackVpnConfig() &&
                    _setup->getFallbackVpnConfig()->isValid())
            {
                LIBENCLOUD_DBG("[Cloud] Retrying with fallback configuration");
                _restart(true, true);
            }
            else
            {
                emit error(Error(Error::CodeClientTimeout));
                _restart();
            }
            break;

        default:
            msg = VpnManager::errorString(err);
            if (errMsg != "")
                msg += "\n\n" + errMsg;

            emit error(Error(Error::CodeClientFailure, msg));
            break;
    }
}

void Cloud::_onRetry ()
{
    _vpnClient->start(_isFallback);
}

//
// private methods
//

// *ECE always retries automatically, QCC does only if forced
void Cloud::_restart (bool fallback, bool force)
{
//    LIBENCLOUD_DBG("fallback: " << fallback << ", force: " << force);

    _isFallback = fallback;

    if (_cfg->config.autoretry)
        force = true;

    if (force)
    {
        stop();
        _retry.start();
    }
    
    // QCC core does full stop()
}

} // namespace libencloud
