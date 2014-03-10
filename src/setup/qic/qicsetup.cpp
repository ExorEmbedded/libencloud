#define LIBENCLOUD_DISABLE_TRACE  // disable heavy tracing
#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include <setup/qic/qicsetup.h>

namespace libencloud {

QicSetup::QicSetup (Config *cfg)
    : SetupInterface(cfg)
    , _backoff(1)
{
    LIBENCLOUD_TRACE;

    _initMsg(_setupMsg);
    connect(&_setupMsg, SIGNAL(error(QString)),
            this, SLOT(_onError(QString)));
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

// TODO
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

void QicSetup::_onError (QString msg)
{
    QState *state = qobject_cast<QState *>(sender());
    int secs = qPow(LIBENCLOUD_RETRY_BASE, _backoff);

    LIBENCLOUD_DBG("state: " << state << ", retrying in " <<
            QString::number(secs) << " seconds");

    QTimer::singleShot(secs * 1000, this, SLOT(_onRetryTimeout()));
    _backoff++;

    emit error(msg);
}

void QicSetup::_onProcessed ()
{
    _backoff = 1;

    emit progress(Progress(tr("Received Configuration from Switchboard"),
                StateReceived, getTotalSteps()));

    emit completed();
}

void QicSetup::_onRetryTimeout ()
{
    LIBENCLOUD_TRACE;

    stop();
    start();
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
