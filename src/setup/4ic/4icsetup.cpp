#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include <setup/4ic/4icsetup.h>

namespace libencloud {

Q4icSetup::Q4icSetup (Config *cfg)
    : SetupInterface(cfg)
    , _backoff(1)
{
    LIBENCLOUD_TRACE;

    emit progress(Progress(tr("Initialising QIC Setup Module"), StateInit, getTotalSteps()));

    _initMsg(_setupMsg);
    connect(&_setupMsg, SIGNAL(error(QString)),
            this, SLOT(_onError(QString)));
    connect(&_setupMsg, SIGNAL(need(QString)),
            this, SIGNAL(need(QString)));
    connect(&_setupMsg, SIGNAL(processed()),
            this, SLOT(_onProcessed()));
    connect(this, SIGNAL(authSupplied(Auth)),
            &_setupMsg, SLOT(authSupplied(Auth)));
}

int Q4icSetup::start ()
{
    LIBENCLOUD_TRACE;

    emit progress(Progress(tr("Retrieving Configuration from Switchboard"),
                StateSetupMsg, getTotalSteps()));
    _setupMsg.process();

    return 0;
}

// TODO
int Q4icSetup::stop ()
{
    LIBENCLOUD_TRACE;

    return 0;
}

const VpnConfig *Q4icSetup::getVpnConfig ()
{
    return _setupMsg.getVpnConfig();
}

int Q4icSetup::getTotalSteps() const
{
    return StateLast - StateFirst + 1;
}

//
// private slots
//

void Q4icSetup::_onError (QString msg)
{
    LIBENCLOUD_DBG("backoff: " << QString::number(_backoff));

    QTimer::singleShot(qPow(LIBENCLOUD_RETRY_TIMEOUT, _backoff) * 1000,
            this, SLOT(_onRetryTimeout()));
    _backoff++;

    emit error(msg);
}

void Q4icSetup::_onProcessed ()
{
    _backoff = 1;

    emit completed();
}

void Q4icSetup::_onRetryTimeout ()
{
    LIBENCLOUD_TRACE;

    stop();
    start();
}

//
// private methods
//

int Q4icSetup::_initMsg (MessageInterface &msg)
{
    msg.setConfig(_cfg);
    msg.setClient(&_client);

    return 0;
}

} // namespace libencloud
