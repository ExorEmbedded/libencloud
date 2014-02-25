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

    _host = vm["ip"].toString();
    LIBENCLOUD_ERR_IF (_host.isEmpty());

    _port = vm["port"].toInt();
    LIBENCLOUD_ERR_IF (_port <= 0);

    _proto = vm["proto"].toString();
    LIBENCLOUD_ERR_IF (_proto.isEmpty());

    _type = vm["type"].toString();
    LIBENCLOUD_ERR_IF (_type.isEmpty());

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
    
    _caPath = cfg->config.sslOp.caPath.absoluteFilePath();
    _certPath = cfg->config.sslOp.certPath.absoluteFilePath();
    _keyPath = cfg->config.sslOp.keyPath.absoluteFilePath();

    return 0;
}

void VpnConfig::setCaPath (const QString &path)
{
    _caPath = path;
}

void VpnConfig::setCertPath (const QString &path)
{
    _certPath = path;
}

void VpnConfig::setKeyPath (const QString &path)
{
    _keyPath = path;
}

QString VpnConfig::toString () const
{
    QString s;
    QTextStream out(&s);

    out << "mode: " << _mode << ", ";
    out << "host: " << _host << ", ";
    out << "port: " << QString::number(_port) << ", ";
    out << "proto: " << _proto << ", ";
    out << "type: " << _type << ", ";
    out << "caPath: " << _caPath << ", ";
    out << "certPath: " << _certPath << ", ";
    out << "keyPath: " << _keyPath;

    return s;
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

        out << _mode << endl;
        out << "remote" << ' ' << _host << endl;
        out << "port" << ' ' << QString::number(_port) << endl;
        if (_proto != "null")
            out << "proto" << ' ' << _proto << endl;
        if (_type != "null")
            out << "dev" << ' ' << _type << endl;
        if (_caPath != "")
            out << "ca" << ' ' << '"' << _caPath << '"' << endl;
        if (_certPath != "")
            out << "cert" << ' ' << '"' << _certPath << '"' << endl;
        if (_keyPath != "")
            out << "key" << ' ' << '"' << _keyPath << '"' << endl;
    }

    LIBENCLOUD_ERR_IF (!configFile.open(QIODevice::WriteOnly));
    LIBENCLOUD_ERR_IF (configFile.write(s.toAscii()) < 0);
    configFile.close();

    return 0;
err:
    configFile.close();
    return ~0;
}

//
// private methods
//

int VpnConfig::init ()
{
    _mode = "client";

    return 0;
}

} // namespace libencloud
