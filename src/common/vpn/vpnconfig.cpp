#include <common/common.h>
#include <common/config.h>
#include <common/vpn/vpnconfig.h>

namespace libencloud {

//
// public methods
//

VpnConfig::VpnConfig ()
    : _valid(true)
{
    LIBENCLOUD_TRACE;

    init();
}

// From ECE-style map with overrides from local configuration
VpnConfig::VpnConfig (const QVariantMap &vm, Config *cfg)
    : _valid(true)
{
    LIBENCLOUD_TRACE;

    init();
    LIBENCLOUD_ERR_IF (fromMap(vm));
    LIBENCLOUD_ERR_IF (fromCfg(cfg));

    return;
err:
    _valid = false;
    return ;
}

// From QIC-style string with overrides from local configuration
VpnConfig::VpnConfig (const QString &s, Config *cfg)
    : _valid(true)
{
    LIBENCLOUD_TRACE;

    init();

    LIBENCLOUD_ERR_IF (fromString(s));
    LIBENCLOUD_ERR_IF (fromCfg(cfg));

    return;
err:
    _valid = false;
    return ;
}

bool VpnConfig::isValid () const
{
    return _valid;
}

int VpnConfig::fromMap (const QVariantMap &vm)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (!vm.contains("ip"));
    _data["remote"] << vm["ip"].toString();

    LIBENCLOUD_ERR_IF (!vm.contains("port"));
    _data["port"] << vm["port"].toString();

    LIBENCLOUD_ERR_IF (!vm.contains("proto"));
    _data["proto"] << vm["proto"].toString();

    LIBENCLOUD_ERR_IF (!vm.contains("type"));
    _data["dev"] << vm["type"].toString();

    _valid = true;
    return 0;
err:
    _valid = false;
    return ~0;
}

// TODO validate
int VpnConfig::fromString (const QString &s)
{
    LIBENCLOUD_TRACE;

    _string = s;

    return 0;
}

int VpnConfig::fromCfg (Config *cfg)
{
    if (cfg == NULL)
        return 0;
    
    _data["ca"] << cfg->config.sslOp.caPath.absoluteFilePath();
    _data["cert"] << cfg->config.sslOp.certPath.absoluteFilePath();
    _data["key"] << cfg->config.sslOp.keyPath.absoluteFilePath();

    return 0;
}

QString VpnConfig::toString () const
{
    QString s;
    QTextStream out(&s);

    if (_data.contains("client"))
        out << "client: yes, ";
    out << "remote: " << _data["remote"].join(" ") << ", ";
    out << "port: " << _data["port"].join(" ") << ", ";
    out << "proto: " << _data["proto"].join(" ") << ", ";
    out << "type: " << _data["dev"].join(" ") << ", ";
    out << "caPath: " << _data["ca"].join(" ") << ", ";
    out << "certPath: " << _data["cert"].join(" ") << ", ";
    out << "keyPath: " << _data["key"].join(" ");

    return s;
}

int VpnConfig::fromFile (const QString &path)
{
    QByteArray ba;
    QFile f(path);

    LIBENCLOUD_ERR_IF (!f.open(QIODevice::ReadOnly));
    LIBENCLOUD_ERR_IF ((ba = f.readAll()).isEmpty());

    foreach (QString line, ba.split('\n')) 
    {
        QStringList params = line.split(QRegExp("\\s"));

        if (params.count() < 1)
            continue;

        QString key = params[0];
        params.removeFirst();

        _data[key] = params;
    }

    f.close();

    return 0;
err:
    f.close();
    return ~0;
}

int VpnConfig::toFile (const QString &path) const
{
    QString s;
    QFile configFile(path);

    if (_string != "")  // created from string
    {
        s = _string;
    }
    else  // created from map
    {
        QTextStream out(&s);

        if (_data.contains("client"))
            out << "client" << endl;
        out << "remote" << ' ' << _data["remote"].join(" ") << endl;
        out << "port" << ' ' << _data["port"][0] << endl;
        if (_data.contains("proto"))
            out << "proto" << ' ' << _data["proto"][0] << endl;
        if (_data.contains("dev"))
            out << "dev" << ' ' << _data["dev"][0] << endl;
        if (_data.contains("ca"))
            out << "ca" << ' ' << '"' << _data["ca"][0] << '"' << endl;
        if (_data.contains("cert"))
            out << "cert" << ' ' << '"' << _data["cert"][0] << '"' << endl;
        if (_data.contains("key"))
            out << "key" << ' ' << '"' << _data["key"][0] << '"' << endl;
    }

    LIBENCLOUD_ERR_IF (!configFile.open(QIODevice::WriteOnly));
    LIBENCLOUD_ERR_IF (configFile.write(s.toAscii()) < 0);
    configFile.close();

    return 0;
err:
    configFile.close();
    return ~0;
}

QStringList VpnConfig::get (const QString &key) const
{
    if (!_data.contains(key))
        return QStringList();

    return _data[key];
}

//
// private methods
//

int VpnConfig::init ()
{
    _data["client"] = QStringList();

    return 0;
}

} // namespace libencloud
