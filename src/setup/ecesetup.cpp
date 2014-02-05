#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include "ecesetup.h"

namespace libencloud {

EceSetup::EceSetup ()
{
    LIBENCLOUD_TRACE;
}

int EceSetup::start ()
{
    LIBENCLOUD_TRACE;

    retrSbInfo();

    return 0;
}

int EceSetup::retrSbInfo ()
{
    LIBENCLOUD_TRACE;

    QTimer::singleShot(3000, this, SLOT(timeout()));

    return 0;
}

//
// private slots
//

void EceSetup::timeout ()
{
    LIBENCLOUD_TRACE;

    emit stateChanged (tr("new state"));
}

} // namespace libencloud
