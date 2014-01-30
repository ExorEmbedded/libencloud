#ifndef _LIBENCLOUD_PRIV_CONTEXT_H_
#define _LIBENCLOUD_PRIV_CONTEXT_H_

#include <QtCore/QCoreApplication>
#include <QTimer>
#include <encloud/core.h>
#include "thread.h"
#include "client.h"
#include "vpn.h"
#include "crypto.h"
#include "config.h"

#if 0
/** \brief SB info object */
struct libencloud_info_s
{
    bool license_valid;
    time_t license_expiry;
};
typedef struct libencloud_info_s libencloud_info_t;
#endif

namespace libencloud {

/** \brief LIBENCLOUD internal object */
class Context : public QObject
{
    Q_OBJECT;

public:
    Context ();
    ~Context ();

    libencloud_rc init (int argc, char *argv[]);

    libencloud_rc setStateCb (libencloud_state_cb stateCb, void *arg);

    libencloud_rc start ();
    libencloud_rc stop ();

#ifndef LIBENCLOUD_TYPE_SECE
    const char *getSerial () const;
    const char *getPoi () const;
#endif

signals:
    void started();
    void stopped();
    void aborted();

public:
    const Config *getConfig () const; 

private:
    void term ();

    bool inited;
    QCoreApplication *app;

#ifndef LIBENCLOUD_TYPE_SECE
    char *serial;
    char *poi;
#endif

    libencloud::Client client;
    libencloud::Config cfg;

    libencloud_crypto_t crypto;
    libencloud_vpn_conf_t conf;

    QThread thread;
    libencloud::Worker worker;
    QTimer timer;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_CONTEXT_H_ */
