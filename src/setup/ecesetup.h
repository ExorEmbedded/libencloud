#ifndef _LIBENCLOUD_PRIV_SETUP_ECE_H_
#define _LIBENCLOUD_PRIV_SETUP_ECE_H_

#include <QtPlugin>
#include <setup/setup.h>

namespace libencloud {

class EceSetup : public QObject, public SetupInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::SetupInterface)

public:
    EceSetup ();

    int start ();

signals:
    void stateChanged (const QString &state);

private slots:
    void timeout();

private:
    int retrSbInfo();
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_ECE_H_ */
