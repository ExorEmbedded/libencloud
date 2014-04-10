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
{
}

Auth::Auth (Id id, Type type, QString url, 
        QString user, QString pass)
    : _valid(false)
{
    if (setId(id) ||
            setType(type) ||
            setUrl(url) ||
            setUser(user) ||
            setPass(pass))
        goto err;

    _valid = true;

err:
    return; 
}

Auth::Auth (const QString &id, QString type, QString url, 
        QString user, QString pass)
    : _valid(false)
{
    if (setStrId(id) ||
            setStrType(type) ||
            setUrl(url) ||
            setUser(user) ||
            setPass(pass))
        goto err;

    _valid = true;

err:
    return; 
}

bool Auth::isValid () const
{
    return _valid;
}

QString Auth::toString () const
{
    QString s;

    s += "id: " + getStrId() + ", ";
    s += "type: " + getStrType() + ", ";
    s += "url: " + getUrl() + ", ";
    s += "user: " + getUser() + ", ";
    s += "pass: <not shown>";

    return s;
}

Auth::Id Auth::getId () const
{
    return _id;
}

int Auth::setId (Auth::Id id)
{
    if (id < FirstId || id > LastId)
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
    if (type < FirstType || type > LastType)
        return ~0;
    
    _type = type;

    return 0;
}

const QString Auth::getStrType () const
{
    switch (_type) 
    {
        case Auth::HttpType:
            return "http";
        case Auth::SocksType:
            return "socks";
        default:
            return "";
    }
}

int Auth::setStrType (const QString &type)
{
    if (type == "")
        _type = Auth::NoneType;
    else if (type == "http")
        _type = Auth::HttpType;
    else if (type == "socks")
        _type = Auth::SocksType;
    else
        return ~0;

    return 0;
}

Auth::Type Auth::typeFromQt (QNetworkProxy::ProxyType type)
{
    switch (type)
    {
        case QNetworkProxy::HttpProxy:
            return Auth::HttpType;
        case QNetworkProxy::Socks5Proxy:
            return Auth::SocksType;
        default:
            return Auth::NoneType;
    }
}

const QString &Auth::getUrl () const
{
    return _url;
}

int Auth::setUrl (const QString &url)
{
    if (!QUrl(url).isValid())
        return ~0;

    _url = url;

    return 0;
}

const QString &Auth::getUser () const
{
    return _user;
}

int Auth::setUser (const QString &user)
{
    if (user == "")
        return ~0;

    _user = user;

    return 0;
}

const QString &Auth::getPass () const
{
    return _pass;
}

int Auth::setPass (const QString &pass)
{
    if (pass == "")
        return ~0;

    _pass = pass;

    return 0;
}

} // namespace libencloud
