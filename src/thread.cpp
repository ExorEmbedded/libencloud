#include "common.h"
#include "config.h"
#include "thread.h"

namespace libencloud {

//
// public methods
//

Worker::Worker ()
    : _stopped(true)
    , _aborted(false)
    , _state(LIBENCLOUD_ST_INITIAL)
{
    LIBENCLOUD_TRACE;
}

Worker::~Worker ()
{
    LIBENCLOUD_TRACE;

    stop();
}

libencloud_rc Worker::setStateCb (libencloud_state_cb stateCb, void *arg)
{
    _stateCb = stateCb;
    _stateCbArg = arg;

    return LIBENCLOUD_RC_SUCCESS;
}

libencloud_rc Worker::start ()
{
    LIBENCLOUD_TRACE;

    if (!_stopped)
        return LIBENCLOUD_RC_SUCCESS;

    LIBENCLOUD_TRACE;

    _mutex.lock();
    _stopped = false;
    _mutex.unlock();

    step();

    return LIBENCLOUD_RC_SUCCESS;
}

libencloud_rc Worker::stop ()
{
    LIBENCLOUD_TRACE;

    if (_stopped)
        return LIBENCLOUD_RC_SUCCESS;

    _mutex.lock();
    _stopped = true;
    _mutex.unlock();

    return LIBENCLOUD_RC_SUCCESS;
}

libencloud_rc Worker::abort ()
{
    LIBENCLOUD_TRACE;

    if (_aborted)
        return LIBENCLOUD_RC_SUCCESS;

    _mutex.lock();
    _aborted = true;
    _mutex.unlock();

    emit finished();

    return LIBENCLOUD_RC_SUCCESS;
}

//
// private slots
//

void Worker::started ()
{
    start();
}

void Worker::stopped ()
{
    stop();
}

void Worker::aborted ()
{
    abort();
}

void Worker::onTimeout ()
{
    LIBENCLOUD_TRACE;

    if (_aborted) 
        return;

    if (_stopped)
    {
        LIBENCLOUD_DBG("stopped");
    }
    else
    {
        LIBENCLOUD_DBG("running");
    }
}

//
// private methods
//

void Worker::step ()
{
    LIBENCLOUD_TRACE;

    _stateCb(_state, _stateCbArg);

    switch (_state)
    {
        case LIBENCLOUD_ST_INITIALISING:
        case LIBENCLOUD_ST_CONNECTING:
        case LIBENCLOUD_ST_CONNECTED:
        default:
            break;
    }
}

} // namespace libencloud
