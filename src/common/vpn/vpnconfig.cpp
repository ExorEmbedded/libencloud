#include <encloud/Vpn/VpnConfig>
#include <encloud/Utils>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

//
// public methods
//

VpnConfig::VpnConfig ()
{
    LIBENCLOUD_TRACE;

    _valid = false;
    _isFile = false;

    clear();
}

// From ECE-style map
VpnConfig::VpnConfig (const QVariantMap &vm)
{
    LIBENCLOUD_TRACE;

    clear();
    LIBENCLOUD_ERR_IF (fromMap(vm));

    _valid = true;
    _isFile = false;
err:
    return ;
}

// From QCC-style string
VpnConfig::VpnConfig (const QString &s)
{
    LIBENCLOUD_TRACE;

    clear();

    LIBENCLOUD_ERR_IF (fromString(s, false));  // no parsing

    _valid = true;
    _isFile = false;
err:
    return;
}

int VpnConfig::clear ()
{
    _valid = false;
    _isFile = false;
    _string = "";

    _client = true;
    _dev = TapDev;

    _remotePort = PortDefault;
    _remoteProto = UdpProto;

    _fallbackEnabled = false;
    _fallbackRemotePort = PortDefault;
    _fallbackRemoteProto = UdpProto;

    return 0;
}

bool VpnConfig::isValid () const
{
    return _valid;
}

/* Returns true if the configuration is file based */
bool VpnConfig::isFile () const
{
    return _isFile;
}

int VpnConfig::fromMap (const QVariantMap &vm)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_ERR_IF (!vm.contains("ip"));
    LIBENCLOUD_ERR_IF (setRemote(vm["ip"].toString()));

    LIBENCLOUD_ERR_IF (!vm.contains("port"));
    LIBENCLOUD_ERR_IF (setRemotePort(vm["port"].toInt()));

    LIBENCLOUD_ERR_IF (!vm.contains("proto"));
    LIBENCLOUD_ERR_IF (setRemoteProto((VpnConfig::Proto) vm["proto"].toInt()));

    LIBENCLOUD_ERR_IF (!vm.contains("type"));
    LIBENCLOUD_ERR_IF (setDev((VpnConfig::Dev) vm["type"].toInt()));

    _valid = true;
    return 0;
err:
    _valid = false;
    return ~0;
}

int VpnConfig::fromString (const QString &s, bool parse)
{
    LIBENCLOUD_TRACE;

    if (parse)
    {
        QStringList tokens = s.split('\n');
        int connectionCount = 0;

        while (!tokens.isEmpty())
        {
            QString line = tokens.takeFirst();

            // parameters are separated by spaces (except if enclosed by quotes)
            QStringList params = line.split(QRegExp(" +(?=(?:[^\"]*\"[^\"]*\")*[^\"]*$)"));
            int nParams = params.count();

            if (nParams < 1)
                continue;

            QString key = params[0];
            params.removeFirst();
            nParams--;

            if (key == "client")
                ;  // already default behaviour
            else if (key == "dev")
            {
                LIBENCLOUD_ERR_IF (nParams != 1);
                LIBENCLOUD_ERR_IF (setDev(devFromStr(params[0])));
            }
            else if (key == "proto")
            {
                LIBENCLOUD_ERR_IF (nParams != 1);
                LIBENCLOUD_ERR_IF (setRemoteProto(VpnConfig::protoFromStr(params[0])));
                LIBENCLOUD_ERR_IF (setFallbackRemoteProto(VpnConfig::protoFromStr(params[0])));
            }
            else if (key == "remote")
            {
                LIBENCLOUD_ERR_IF (nParams < 1);
                LIBENCLOUD_ERR_IF (setRemote(params[0]));
                if (nParams >= 2)
                    LIBENCLOUD_ERR_IF (setRemotePort(params[1].toInt()));
                if (nParams >= 3)
                    LIBENCLOUD_ERR_IF (setRemoteProto(VpnConfig::protoFromStr(params[2])));
            }
            else if (key == "<connection>")
            {
                line = tokens.takeFirst();
                LIBENCLOUD_ERR_IF (tokens.isEmpty());

                QStringList params = line.split(QRegExp("\\s"));
                nParams = params.count();
                LIBENCLOUD_ERR_IF (nParams < 2);
                LIBENCLOUD_ERR_IF (params[0] != "remote");

                if (connectionCount == 0)
                {
                    LIBENCLOUD_ERR_IF (setRemote(params[1]));
                    if (nParams >= 3)
                        LIBENCLOUD_ERR_IF (setRemotePort(params[2].toInt()));
                    if (nParams >= 4)
                        LIBENCLOUD_ERR_IF (setRemoteProto(VpnConfig::protoFromStr(params[3])));
                }
                else if (connectionCount == 1)
                {
                    LIBENCLOUD_ERR_IF (setFallbackRemote(params[1]));
                    if (nParams >= 3)
                        LIBENCLOUD_ERR_IF (setFallbackRemotePort(params[2].toInt()));
                    if (nParams >= 4)
                        LIBENCLOUD_ERR_IF (setFallbackRemoteProto(VpnConfig::protoFromStr(params[3])));
                    LIBENCLOUD_ERR_IF (setFallbackEnabled(true));
                }
                else
                    LIBENCLOUD_ERR_MSG_IF (1, "Max 2 'connection' blocks supported");
                
                line = tokens.takeFirst();
                LIBENCLOUD_ERR_IF (line != "</connection>");

                connectionCount++;
            }
            else if (key == "ca")
            {
                LIBENCLOUD_ERR_IF (nParams != 1);
                LIBENCLOUD_ERR_IF (setCaPath(params[0].remove('"')));
            }
            else if (key == "cert")
            {
                LIBENCLOUD_ERR_IF (nParams != 1);
                LIBENCLOUD_ERR_IF (setCertPath(params[0].remove('"')));
            }
            else if (key == "key")
            {
                LIBENCLOUD_ERR_IF (nParams != 1);
                LIBENCLOUD_ERR_IF (setKeyPath(params[0].remove('"')));
            }
            else if (key == "pkcs12")
            {
                LIBENCLOUD_ERR_IF (nParams != 1);
                LIBENCLOUD_ERR_IF (setP12Path(params[0].remove('"')));
            }
        }

        LIBENCLOUD_ERR_IF (!_isValid(false));
    }
    else
    {
        _string = s;
    }

    _valid = true;

    return 0;
err:
    return ~0;
}

QString VpnConfig::toString () const
{
    QString s;
    QTextStream out(&s);

    out << "client: " << _client << ", ";
    out << "dev: " << getDev() << ", ";

    out << "remote: " << getRemote() << ", ";
    out << "port: " << getRemotePort() << ", ";
    out << "proto: " << getRemoteProto() << ", ";

    if (isFallbackEnabled())
    {
        out << "fb_remote: " << getFallbackRemote() << ", ";
        out << "fb_port: " << getFallbackRemotePort() << ", ";
        out << "fb_proto: " << getFallbackRemoteProto() << ", ";
    }

    out << "ca_path: " << getCaPath() << ", ";

    out << "cert_path: " << getCertPath() << ", ";
    out << "key_path: " << getKeyPath() << ", ";
    out << "p12_path: " << getP12Path();

    return s;
}

// If (parse == false) no local validation is done and members are not
// populated
int VpnConfig::fromFile (const QString &path, bool parse)
{
    QByteArray ba;
    QFile f(path);

    if (!f.open(QIODevice::ReadOnly) ||
            (ba = f.readAll()).isEmpty() ||
            fromString(ba, parse))
        goto err;

    f.close();

    _isFile = true;

    return 0;
err:
    f.close();
    return ~0;
}

int VpnConfig::toFile (const QString &path) const
{
    QString s;
    QFile configFile(path);

    QTextStream out(&s);

    if (_string != "")  // created from string
    {
        s = _string;
    }
    else  // created manually or from map
    {
        if (_client)
            out << "client" << endl;

        out << "dev" << ' ' << VpnConfig::devToStr(getDev()) << endl;

        out << "<connection>" << endl;
        out << "remote" << ' ' << getRemote() << ' ' << getRemotePort() <<
            ' ' << VpnConfig::protoToStr(getRemoteProto()) << endl;
        out << "</connection>" << endl;

        if (isFallbackEnabled())
        {
            out << "<connection>" << endl;
            out << "remote" << ' ' << getFallbackRemote() << ' ' << getFallbackRemotePort() <<
                ' ' << VpnConfig::protoToStr(getFallbackRemoteProto()) << endl;
            out << "</connection>" << endl;
        }

        out << "ca" << ' ' << '"' << getCaPath() << '"' << endl;

        if (getCertPath() != "")
            out << "cert" << ' ' << '"' << getCertPath() << '"' << endl;
        if (getKeyPath() != "")
            out << "key" << ' ' << '"' << getKeyPath() << '"' << endl;
        if (getP12Path() != "")
            out << "pkcs12" << ' ' << '"' << getP12Path() << '"' << endl;
    }

    LIBENCLOUD_ERR_IF (!configFile.open(QIODevice::WriteOnly));
    LIBENCLOUD_ERR_IF (configFile.write(s.toAscii()) < 0);
    configFile.close();

    return 0;
err:
    configFile.close();
    return ~0;
}

bool VpnConfig::validProto (Proto proto)
{
    return (proto >= VpnConfig::FirstProto && proto <= VpnConfig::LastProto);
}

VpnConfig::Proto VpnConfig::protoFromStr (const QString &s)
{
    if (s == "udp")
        return VpnConfig::UdpProto;
    else if (s == "tcp")
        return VpnConfig::TcpProto;
    else
        return VpnConfig::NoneProto;
}

QString VpnConfig::protoToStr (VpnConfig::Proto proto)
{
    switch (proto)
    {
        case VpnConfig::UdpProto:
            return "udp";
        case VpnConfig::TcpProto:
            return "tcp";
        default:
            return "";
    }
}

VpnConfig::Dev VpnConfig::getDev () const
{
    return _dev;
}

int VpnConfig::setDev (Dev dev)
{
    LIBENCLOUD_ERR_IF (!validDev(dev));

    _dev = dev;

    return 0;
err:
    return ~0;
}

bool VpnConfig::validDev (Dev dev)
{
    return (dev >= VpnConfig::FirstDev && dev <= VpnConfig::LastDev);
}

VpnConfig::Dev VpnConfig::devFromStr (const QString &s)
{
    if (s == "tap")
        return VpnConfig::TapDev;
    else if (s == "tun")
        return VpnConfig::TunDev;
    else
        return VpnConfig::NoneDev;
}

QString VpnConfig::devToStr (VpnConfig::Dev dev)
{
    switch (dev)
    {
        case VpnConfig::TapDev:
            return "tap";
        case VpnConfig::TunDev:
            return "tun";
        default:
            return "";
    }
}

const QString &VpnConfig::getRemote () const
{
    return _remote;
}

int VpnConfig::setRemote (const QString &remote)
{
    LIBENCLOUD_ERR_IF (!VpnConfig::validRemote(remote));

    _remote = remote;

    return 0;
err:
    return ~0;
}

int VpnConfig::getRemotePort () const
{
    return _remotePort;
}

int VpnConfig::setRemotePort (int port)
{
    LIBENCLOUD_ERR_IF (port <= 0 || port > 65535);

    _remotePort = port;

    return 0;
err:
    return ~0;
}

bool VpnConfig::validRemote (const QString &remote)
{
    return (utils::validIp(remote) || utils::validHost(remote));
}

VpnConfig::Proto VpnConfig::getRemoteProto () const
{
    return _remoteProto;
}

int VpnConfig::setRemoteProto (VpnConfig::Proto proto)
{
    LIBENCLOUD_ERR_IF (!validProto(proto));

    _remoteProto = proto;

    return 0;
err:
    return ~0;
}

bool VpnConfig::isFallbackEnabled () const
{
    return _fallbackEnabled;
}

int VpnConfig::setFallbackEnabled (bool enabled)
{
    _fallbackEnabled = enabled;

    return 0;
}

const QString &VpnConfig::getFallbackRemote () const
{
    return _fallbackRemote;
}

int VpnConfig::setFallbackRemote (const QString &remote)
{
    LIBENCLOUD_ERR_IF (!VpnConfig::validRemote(remote));

    _fallbackRemote = remote;

    return 0;
err:
    return ~0;
}

int VpnConfig::getFallbackRemotePort () const
{
    return _fallbackRemotePort;
}

int VpnConfig::setFallbackRemotePort (int port)
{
    LIBENCLOUD_ERR_IF (port <= 0 || port > 65535);

    _fallbackRemotePort = port;

    return 0;
err:
    return ~0;
}

VpnConfig::Proto VpnConfig::getFallbackRemoteProto () const
{
    return _fallbackRemoteProto;
}

int VpnConfig::setFallbackRemoteProto (VpnConfig::Proto proto)
{
    LIBENCLOUD_ERR_IF (!validProto(proto));

    _fallbackRemoteProto = proto;

    return 0;
err:
    return ~0;
}

const QString &VpnConfig::getCaPath () const
{
    return _caPath;
}

int VpnConfig::setCaPath (const QString &caPath)
{
    LIBENCLOUD_ERR_IF (caPath == "");

    _caPath = caPath;

    return 0;
err:
    return ~0;
}

const QString &VpnConfig::getCertPath () const
{
    return _certPath;
}

int VpnConfig::setCertPath (const QString &certPath)
{
    LIBENCLOUD_ERR_IF (certPath == "");

    _certPath = certPath;

    return 0;
err:
    return ~0;
}

const QString &VpnConfig::getKeyPath () const
{
    return _keyPath;
}

int VpnConfig::setKeyPath (const QString &keyPath)
{
    LIBENCLOUD_ERR_IF (keyPath == "");

    _keyPath = keyPath;

    return 0;
err:
    return ~0;
}

const QString &VpnConfig::getP12Path () const
{
    return _p12Path;
}

int VpnConfig::setP12Path (const QString &p12Path)
{
    LIBENCLOUD_ERR_IF (p12Path == "");

    _p12Path = p12Path;

    return 0;
err:
    return ~0;
}

//
// private methods
//

// Basic consistency check
// Note: to be used only for parsed configuration (parse=true)
bool VpnConfig::_isValid (bool strict)
{
    LIBENCLOUD_ERR_IF (!VpnConfig::validDev(getDev()));
    LIBENCLOUD_ERR_IF (!VpnConfig::validProto(getRemoteProto()));
    LIBENCLOUD_ERR_IF (getRemote().isEmpty());

    if (strict)
        LIBENCLOUD_ERR_IF (getCaPath().isEmpty());

    return true;
err:
    return false;
}

} // namespace libencloud
