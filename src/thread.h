#ifndef _LIBENCLOUD_PRIV_THREAD_H_
#define _LIBENCLOUD_PRIV_THREAD_H_

#include <QThread>
#include <QMutex>
#include <encloud/common.h>
#include <encloud/state.h>

namespace libencloud {

class Worker : public QObject
{
    Q_OBJECT

public:
    Worker();
    ~Worker();

    libencloud_rc setStateCb (libencloud_state_cb stateCb, void *arg);

    libencloud_rc start ();
    libencloud_rc stop ();
    libencloud_rc abort ();

signals:
    void finished ();

private slots:
    void started ();
    void stopped ();
    void aborted ();
    void onTimeout ();

private:
    void step ();

    QMutex _mutex;
    bool _stopped;
    bool _aborted;

    libencloud_state _state;
    libencloud_state_cb _stateCb;
    void *_stateCbArg;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_THREAD_H_ */
