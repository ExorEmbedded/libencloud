#include "common.h"
#include "config.h"
#include "thread.h"

namespace encloud {

//
// public methods
//

Worker::Worker ()
    : _stopped(true)
    , _aborted(false)
    , _state(ENCLOUD_ST_INITIAL)
{
    ENCLOUD_TRACE;
}

Worker::~Worker ()
{
    ENCLOUD_TRACE;

    stop();
}

encloud_rc Worker::setStateCb (encloud_state_cb stateCb, void *arg)
{
    _stateCb = stateCb;
    _stateCbArg = arg;

    return ENCLOUD_RC_SUCCESS;
}

encloud_rc Worker::start ()
{
    ENCLOUD_TRACE;

    if (!_stopped)
        return ENCLOUD_RC_SUCCESS;

    ENCLOUD_TRACE;

    _mutex.lock();
    _stopped = false;
    _mutex.unlock();

    step();

    return ENCLOUD_RC_SUCCESS;
}

encloud_rc Worker::stop ()
{
    ENCLOUD_TRACE;

    if (_stopped)
        return ENCLOUD_RC_SUCCESS;

    _mutex.lock();
    _stopped = true;
    _mutex.unlock();

    return ENCLOUD_RC_SUCCESS;
}

encloud_rc Worker::abort ()
{
    ENCLOUD_TRACE;

    if (_aborted)
        return ENCLOUD_RC_SUCCESS;

    _mutex.lock();
    _aborted = true;
    _mutex.unlock();

    emit finished();

    return ENCLOUD_RC_SUCCESS;
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
    ENCLOUD_TRACE;

    if (_aborted) 
        return;

    if (_stopped)
    {
        ENCLOUD_DBG("stopped");
    }
    else
    {
        ENCLOUD_DBG("running");
    }
}

//
// private methods
//

void Worker::step ()
{
    ENCLOUD_TRACE;

    _stateCb(_state, _stateCbArg);

    switch (_state)
    {
        case ENCLOUD_ST_INITIALISING:
        case ENCLOUD_ST_CONNECTING:
        case ENCLOUD_ST_CONNECTED:
        default:
            break;
    }
}

} // namespace encloud
