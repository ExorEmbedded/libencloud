#ifndef _LIBENCLOUD_PRIV_CLOUD_VPNCONFIG_H_
#define _LIBENCLOUD_PRIV_CLOUD_VPNCONFIG_H_

#include <encloud/Utils>
#include <common/config.h>

namespace libencloud {

class VpnConfig
{
public:
    VpnConfig ();
    VpnConfig (const QVariantMap &vm, Config *cfg = NULL);
    VpnConfig (const QString &s, Config *cfg = NULL);

    // use compiler-default
    //VpnConfig &operator=(const VpnConfig vc);

    int clear ();
    bool isValid () const;
    int fromMap (const QVariantMap &vm);
    int fromString (const QString &s);
    int fromCfg (Config *cfg);

    QString toString () const;
    int fromFile (const QString &path);
    int toFile (const QString &path) const;

    QStringList get (const QString &key) const;

private:
    bool _valid;
    QString _string;

    QHash<QString, QStringList> _data;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_CLOUD_VPNCONFIG_H_ */
