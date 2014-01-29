#ifndef _ENCLOUD_PRIV_CONTEXT_H_
#define _ENCLOUD_PRIV_CONTEXT_H_

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
struct encloud_info_s
{
    bool license_valid;
    time_t license_expiry;
};
typedef struct encloud_info_s encloud_info_t;
#endif

namespace encloud {

/** \brief ENCLOUD internal object */
class Context : public QObject
{
    Q_OBJECT;

public:
    Context ();
    ~Context ();

    encloud_rc init (int argc, char *argv[]);

    encloud_rc setStateCb (encloud_state_cb stateCb, void *arg);

    encloud_rc start ();
    encloud_rc stop ();

#ifndef ENCLOUD_TYPE_SECE
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

#ifndef ENCLOUD_TYPE_SECE
    char *serial;
    char *poi;
#endif

    encloud::Client client;
    encloud::Config cfg;

    encloud_crypto_t crypto;
    encloud_vpn_conf_t conf;

    QThread thread;
    encloud::Worker worker;
    QTimer timer;
};

} // namespace encloud

#endif  /* _ENCLOUD_PRIV_CONTEXT_H_ */
