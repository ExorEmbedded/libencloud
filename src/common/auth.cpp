#include <QUrl>
#include <encloud/Auth>
#include <common/config.h>
#include <common/common.h>

namespace libencloud {

//
// public methods
//

Auth::Auth ()
    : _valid(false)
    , _id(Auth::NoneId)
    , _type(Auth::NoneType)
{
}

Auth::Auth (Id id, Type type, QString url, 
        QString user, QString pass, QString path, QString p12Pass)
    : _valid(false)
    , _id(Auth::NoneId)
    , _type(Auth::NoneType)
{
    LIBENCLOUD_ERR_IF (setId(id));
    LIBENCLOUD_ERR_IF (setType(type));
    LIBENCLOUD_ERR_IF (setUrl(url));
    LIBENCLOUD_ERR_IF (setUser(user));
    LIBENCLOUD_ERR_IF (setPass(pass));
    LIBENCLOUD_ERR_IF (setPath(path));
    LIBENCLOUD_ERR_IF (setP12Pass(p12Pass));

    _valid = true;

err:
    return; 
}

Auth::Auth (const QString &id, QString type, QString url,
        QString user, QString pass, QString path, QString p12Pass)
    : _valid(false)
    , _id(Auth::NoneId)
    , _type(Auth::NoneType)
{
    LIBENCLOUD_ERR_IF (setStrId(id));
    LIBENCLOUD_ERR_IF (setStrType(type));
    LIBENCLOUD_ERR_IF (setUrl(url));
    LIBENCLOUD_ERR_IF (setUser(user));
    LIBENCLOUD_ERR_IF (setPass(pass));
    LIBENCLOUD_ERR_IF (setPath(path));
    LIBENCLOUD_ERR_IF (setP12Pass(p12Pass));

    _valid = true;

err:
    return; 
}

bool Auth::isValid () const
{
    return _valid;
}

// Use this to mark an initially invalid (e.g. default constructor) Auth object
// as valid. Minimum requirements are: valid id, type, url (may be empty)
// user (not for CertificateType) and password.
int Auth::validate ()
{
    LIBENCLOUD_ERR_IF (!isIdValid(_id));
    LIBENCLOUD_ERR_IF (!isTypeValid(_type));
    LIBENCLOUD_ERR_IF (!isUrlValid(_url));
    LIBENCLOUD_ERR_IF (!isUserValid(_user));
    LIBENCLOUD_ERR_IF (!isPassValid(_pass));
    LIBENCLOUD_ERR_IF (!isPathValid(_path));
    LIBENCLOUD_ERR_IF (!isP12PassValid(_p12Pass));

    _valid = true;

    return 0;
err:
    return ~0;
}

QString Auth::toString () const
{
    QString s;

    s += "valid: " +    QString::number(isValid()) + ", ";
    s += "id: " +       getStrId() + ", ";
    s += "type: " +     getStrType() + ", ";
    s += "url: " +      getUrl() + ", ";
    s += "user: " +     getUser() + ", ";
    s += "path: " +     getPath();

    return s;
}

bool Auth::operator== (const libencloud::Auth &auth) const
{
    return (_id == auth._id &&
            _type == auth._type &&
            _url == auth._url &&
            _user == auth._user &&
            _pass == auth._pass &&
            _path == auth._path &&
            _p12Pass == auth._p12Pass);
}

QDebug operator<< (QDebug d, const Auth &auth)
{
    d << auth.toString();
    return d;
}

Auth::Id Auth::getId () const
{
    return _id;
}

int Auth::setId (Auth::Id id)
{
    if (!isIdValid(id))
        return ~0;

    _id = id;

    return 0;
}

const QString Auth::getStrId () const
{
    switch (_id) 
    {
        case Auth::SwitchboardId:
            return "sb";
        case Auth::ProxyId:
            return "proxy";
        default:
            return "";
    }
}

int Auth::setStrId (const QString &id)
{
    if (id == "sb")
        _id = Auth::SwitchboardId;
    else if (id == "proxy")
        _id = Auth::ProxyId;
    else
        return ~0;

    return 0;
}

Auth::Type Auth::getType () const
{
    return _type;
}

int Auth::setType (Type type)
{
    if (!isTypeValid(type))
        return ~0;
    
    _type = type;

    return 0;
}

const QString Auth::getStrType () const
{
    switch (_type) 
    {
        case Auth::NoneType:
            return "none";
        case Auth::UrlType:
            return "url";
        case Auth::UserpassType:
            return "user-pass";
        case Auth::CertificateType:
            return "x509";
        case Auth::CertificateUserpassType:
            return "x509-user-pass";
        case Auth::HttpProxyType:
            return "http";
        case Auth::SocksProxyType:
            return "socks";
        default:
            return "";
    }
}

int Auth::setStrType (const QString &type)
{
    if (type == "" || type == "none")
        _type = Auth::NoneType;
    else if (type == "url")
        _type = Auth::UrlType;
    else if (type == "user-pass")
        _type = Auth::UserpassType;
    else if (type == "x509")
        _type = Auth::CertificateType;
    else if (type == "x509-user-pass")
        _type = Auth::CertificateUserpassType;
    else if (type == "http")
        _type = Auth::HttpProxyType;
    else if (type == "socks")
        _type = Auth::SocksProxyType;
    else
        return ~0;

    return 0;
}

Auth::Type Auth::typeFromQt (QNetworkProxy::ProxyType type)
{
    switch (type)
    {
        case QNetworkProxy::HttpProxy:
            return Auth::HttpProxyType;
        case QNetworkProxy::Socks5Proxy:
            return Auth::SocksProxyType;
        default:
            return Auth::NoneType;
    }
}

QNetworkProxy::ProxyType Auth::typeToQt (Type type)
{
    switch (type)
    {
        case Auth::HttpProxyType:
            return QNetworkProxy::HttpProxy;
        case Auth::SocksProxyType:
            return QNetworkProxy::Socks5Proxy;
        default:
            return QNetworkProxy::NoProxy;
    }
}

const QString &Auth::getUrl () const
{
    return _url;
}

int Auth::setUrl (const QString &url)
{
    // url can be unspecified (e.g. file-based VPN configuration)
    /*
    if (!isUrlValid(url))
        return ~0;
    */

    _url = url;

    return 0;
}

const QString &Auth::getUser () const
{
    return _user;
}

int Auth::setUser (const QString &user)
{
    if (!isUserValid(user))
        return ~0;

    _user = user;

    return 0;
}

const QString &Auth::getPass () const
{
    return _pass;
}

/* Password-based auth */
int Auth::setPass (const QString &pass)
{
    if (!isPassValid(pass))
        return ~0;

    _pass = pass;

    return 0;
}

const QString &Auth::getPath () const
{
    return _path;
}

/* Password-based auth or password to decrypt PKCS12 */
int Auth::setPath (const QString &path)
{
    // path can be unspecified (e.g. file-based VPN configuration)
    /*
    if (!isPathValid(path))
        return ~0;
    */

    _path = path;

    return 0;
}

const QString &Auth::getP12Pass () const
{
    return _p12Pass;
}

/* Password to decrypt PKCS12 certs */
int Auth::setP12Pass (const QString &p12Pass)
{
    if (!isP12PassValid(p12Pass))
        return ~0;

    _p12Pass = p12Pass;

    return 0;
}

} // namespace libencloud
