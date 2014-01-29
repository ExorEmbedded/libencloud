#ifndef _ENCLOUD_PRIV_THREAD_H_
#define _ENCLOUD_PRIV_THREAD_H_

#include <QThread>
#include <QMutex>
#include <encloud/common.h>
#include <encloud/state.h>

namespace encloud {

class Worker : public QObject
{
    Q_OBJECT

public:
    Worker();
    ~Worker();

    encloud_rc setStateCb (encloud_state_cb stateCb, void *arg);

    encloud_rc start ();
    encloud_rc stop ();
    encloud_rc abort ();

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

    encloud_state _state;
    encloud_state_cb _stateCb;
    void *_stateCbArg;
};

} // namespace encloud

#endif  /* _ENCLOUD_PRIV_THREAD_H_ */
