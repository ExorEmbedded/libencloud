#include <common/common.h>
#include <common/config.h>
#include "cloud.h"

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
    connect(_vpnClient, SIGNAL(stateChanged(VpnClient::State)), this, SLOT(_vpnStateChanged(VpnClient::State)));

    // authentication requests routed from manager
    connect(_vpnManager, SIGNAL(authRequest()), this, SIGNAL(authRequest()));
    connect(_vpnManager, SIGNAL(proxyAuthRequest()), this, SIGNAL(proxyAuthRequest()));

    // local error handling
    connect(_vpnClient, SIGNAL(sigError(VpnClient::Error, QString)), 
            this, SLOT(_vpnClientErr(VpnClient::Error, QString)));
    connect(_vpnManager, SIGNAL(sigError(VpnManager::Error, QString)), 
            this, SLOT(_vpnManagerErr(VpnManager::Error, QString)));

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

    _vpnManager->detach();
    _vpnClient->stop();

	return 0;
}

//
// private slots
//

void Cloud::_vpnStateChanged (VpnClient::State state)
{
    QString stateStr = VpnClient::stateString(state);

    LIBENCLOUD_DBG("state: " << stateStr);

    switch (VpnClient::StateStarted)
    {
        case VpnClient::StateStarted:
            _vpnManager->attach(_vpnClient, LIBENCLOUD_VPN_MGMT_HOST,
                    _cfg->config.vpnMgmtPort);
            break;
    }

    emit stateChanged(stateStr);
}

void Cloud::_vpnClientErr (VpnClient::Error err, const QString &errMsg)
{
    LIBENCLOUD_DBG("err: " << VpnClient::errorString(err) << " msg: " << errMsg);
}

void Cloud::_vpnManagerErr (VpnManager::Error err, const QString &errMsg)
{
    LIBENCLOUD_DBG("err: " << VpnManager::errorString(err) << " msg: " << errMsg);
}

} // namespace libencloud
