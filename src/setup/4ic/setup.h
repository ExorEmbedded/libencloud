#ifndef _LIBENCLOUD_PRIV_SETUP_4IC_H_
#define _LIBENCLOUD_PRIV_SETUP_4IC_H_

#include <QtPlugin>
#include <setup/setup.h>

namespace libencloud {

class Q4icSetup : public QObject, public SetupInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::SetupInterface)

public:
    Q4icSetup ();
    int init ();
    int start ();
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_4IC_H_ */
