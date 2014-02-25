#ifndef _LIBENCLOUD_PRIV_CLOUD_VPNCONFIG_H_
#define _LIBENCLOUD_PRIV_CLOUD_VPNCONFIG_H_

#include <common/utils.h>
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

    bool isValid () const;
    int fromMap (const QVariantMap &vm);
    int fromString (const QString &s);
    int fromCfg (Config *cfg);

    void setCaPath (const QString &path);
    void setCertPath (const QString &path);
    void setKeyPath (const QString &path);

    QString toString () const;
    int toFile (const QString &path) const;
    
private:
    int init ();

    bool _valid;
    QString _string;

    QString _mode;
    QString _host;
    int _port;
    QString _proto;
    QString _type;

    QString _caPath;
    QString _certPath;
    QString _keyPath;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_CLOUD_VPNCONFIG_H_ */
