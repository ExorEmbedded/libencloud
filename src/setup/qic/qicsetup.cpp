#define LIBENCLOUD_DISABLE_TRACE  // disable heavy tracing
#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include <setup/qic/qicsetup.h>

namespace libencloud {

/**
 * Unlike ECE/SECE Setup, the QIC Setup Module makes no automatic retries =>
 * errors are forwarded to user.
 */

QicSetup::QicSetup (Config *cfg)
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
}

int QicSetup::start ()
{
    LIBENCLOUD_TRACE;

    if (_setupMsg.process())
        return ~0;

    emit progress(Progress(tr("Retrieving Configuration from Switchboard"),
                StateSetupMsg, getTotalSteps()));

    return 0;
}

int QicSetup::stop ()
{
    LIBENCLOUD_TRACE;

    return 0;
}

const VpnConfig *QicSetup::getVpnConfig ()
{
    return _setupMsg.getVpnConfig();
}

int QicSetup::getTotalSteps() const
{
    return StateLast - StateFirst + 1;
}

//
// private slots
//

void QicSetup::_onProcessed ()
{
    LIBENCLOUD_TRACE;

    emit progress(Progress(tr("Received Configuration from Switchboard"),
                StateReceived, getTotalSteps()));

    emit completed();
}

//
// private methods
//

int QicSetup::_initMsg (MessageInterface &msg)
{
    msg.setConfig(_cfg);
    msg.setClient(&_client);

    return 0;
}

} // namespace libencloud
