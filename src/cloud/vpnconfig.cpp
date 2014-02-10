#include <common/common.h>
#include <common/config.h>
#include <cloud/vpnconfig.h>

namespace libencloud {

//
// public methods
//

VpnConfig::VpnConfig ()
    : _valid(true)
{
    LIBENCLOUD_TRACE;
}

VpnConfig::VpnConfig (const QVariantMap &vm, Config *cfg)
    : _valid(true)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (fromMap(vm));

    if (cfg)
    {
        _caPath = cfg->config.sslOp.caPath.absoluteFilePath();
        _certPath = cfg->config.sslOp.certPath.absoluteFilePath();
        _keyPath = cfg->config.sslOp.keyPath.absoluteFilePath();
    }

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
    QTextStream out(&s);
    QFile configFile(path);

    out << "remote" << ' ' << _host << endl;
    out << "port" << ' ' << QString::number(_port) << endl;
    if (_proto != "null")
        out << "proto" << ' ' << _proto << endl;
    if (_type != "null")
        out << "type" << ' ' << _type << endl;
    if (_caPath != "")
        out << "ca" << ' ' << '"' << _caPath << '"' << endl;
    if (_certPath != "")
        out << "cert" << ' ' << '"' << _certPath << '"' << endl;
    if (_keyPath != "")
        out << "key" << ' ' << '"' << _keyPath << '"' << endl;

    LIBENCLOUD_ERR_IF (!configFile.open(QIODevice::WriteOnly));
    LIBENCLOUD_ERR_IF (configFile.write(s.toAscii()) < 0);
    configFile.close();

    return 0;
err:
    configFile.close();
    return ~0;
}

} // namespace libencloud
