#define LIBENCLOUD_DISABLE_TRACE  // disable heavy tracing
#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include <setup/qcc/qccsetup.h>

// Login/Logout are unnecessary when using Basic Auth, and logout doesn't kick
// out of VPN connection which is what we really would need
//#define LIBENCLOUD_SETUP_QCC_WITH_LOGIN

namespace libencloud {

/**
 * Unlike ECE/SECE Setup, the QCC Setup Module makes no automatic retries =>
 * errors are forwarded to user.
 */

QccSetup::QccSetup (Config *cfg)
    : SetupInterface(cfg)
{
    LIBENCLOUD_TRACE;

    _initMsg(_setupMsg);
    connect(&_setupMsg, SIGNAL(error(libencloud::Error)),
            this, SIGNAL(error(libencloud::Error)));
    connect(&_setupMsg, SIGNAL(need(QString)),
            this, SIGNAL(need(QString)));
    connect(&_setupMsg, SIGNAL(processed()),
            this, SLOT(_onProcessed()));
    connect(&_setupMsg, SIGNAL(authRequired(Auth::Id)),
            this, SIGNAL(authRequired(Auth::Id)));
    connect(this, SIGNAL(authSupplied(Auth)),
            &_setupMsg, SLOT(authSupplied(Auth)));
    connect(&_setupMsg, SIGNAL(serverConfigSupply(QVariant)),
            this, SIGNAL(serverConfigSupply(QVariant)));

#ifdef LIBENCLOUD_SETUP_QCC_WITH_LOGIN
    _initMsg(_loginMsg);
    connect(&_loginMsg, SIGNAL(error(libencloud::Error)),
            this, SIGNAL(error(libencloud::Error)));
    connect(this, SIGNAL(authSupplied(Auth)),
            &_loginMsg, SLOT(authSupplied(Auth)));
#endif
}

int QccSetup::start ()
{
    LIBENCLOUD_TRACE;

    _clear();

    QVariantMap data;
    data["in"] = true;

#ifdef LIBENCLOUD_SETUP_QCC_WITH_LOGIN
    _loginMsg.setData(data);
    _loginMsg.process();
#endif

    if (_setupMsg.process())
        return ~0;

    emit progress(Progress(tr("Retrieving Configuration from Switchboard"),
                StateSetupMsg, getTotalSteps()));

    return 0;
}

int QccSetup::stop ()
{
    LIBENCLOUD_TRACE;

    _clear();
    _setupMsg.clear();
    _initMsg(_setupMsg);

    QVariantMap data;
    data["in"] = false;

#ifdef LIBENCLOUD_SETUP_QCC_WITH_LOGIN
    _loginMsg.setData(data);
    _loginMsg.process();
#endif

    return 0;
}

const VpnConfig *QccSetup::getVpnConfig () const
{
    return _setupMsg.getVpnConfig();
}

const VpnConfig *QccSetup::getFallbackVpnConfig () const
{
    return _setupMsg.getFallbackVpnConfig();
}

int QccSetup::getTotalSteps() const
{
    return StateLast - StateFirst + 1;
}

//
// private slots
//

void QccSetup::_onProcessed ()
{
    LIBENCLOUD_TRACE;

    emit progress(Progress(tr("Received Configuration from Switchboard"),
                StateReceived, getTotalSteps()));

    emit completed();
}

//
// private methods
//

int QccSetup::_initMsg (MessageInterface &msg)
{
    msg.setConfig(_cfg);

    return 0;
}

// Clear all generated data
void QccSetup::_clear ()
{
    if (QFile::exists(_cfg->config.sslOp.caPath.absoluteFilePath()))
        LIBENCLOUD_ERR_IF (!QFile::remove(_cfg->config.sslOp.caPath.absoluteFilePath()));
    if (QFile::exists(_cfg->config.vpnConfPath.absoluteFilePath()))
        LIBENCLOUD_ERR_IF (!QFile::remove(_cfg->config.vpnConfPath.absoluteFilePath()));
    if (QFile::exists(_cfg->config.fallbackVpnConfPath.absoluteFilePath()))
        LIBENCLOUD_ERR_IF (!QFile::remove(_cfg->config.fallbackVpnConfPath.absoluteFilePath()));
err:
    return;
}

} // namespace libencloud
