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

Auth::Auth (Id id, Type type, const QString &url, 
        const QString &user, const QString &pass)
    : _valid(false)
{
    LIBENCLOUD_ERR_IF (setId(id));
    LIBENCLOUD_ERR_IF (setType(type));
    LIBENCLOUD_ERR_IF (setUrl(url));
    LIBENCLOUD_ERR_IF (setUser(user));
    LIBENCLOUD_ERR_IF (setPass(pass));

    _valid = true;

err:
    return; 
}

Auth::Auth (const QString &id, const QString &type, const QString &url, 
        const QString &user, const QString &pass)
    : _valid(false)
{
    LIBENCLOUD_ERR_IF (setStrId(id));
    LIBENCLOUD_ERR_IF (setStrType(type));
    LIBENCLOUD_ERR_IF (setUrl(url));
    LIBENCLOUD_ERR_IF (setUser(user));
    LIBENCLOUD_ERR_IF (setPass(pass));

    _valid = true;

err:
    return; 
}

bool Auth::isValid () const
{
    return _valid;
}

Auth::Id Auth::getId () const
{
    return _id;
}

int Auth::setId (Auth::Id id)
{
    LIBENCLOUD_RETURN_IF (id < FirstId || id > LastId, ~0);

    _id = id;

    return 0;
}

int Auth::setStrId (const QString &id)
{
    if (id == "sb")
        _id = Auth::SwitchboardId;
    else if (id == "proxy")
        _id = Auth::ProxyId;
    else
        LIBENCLOUD_ERR_IF (1);

    return 0;
err:
    return ~0;
}

Auth::Type Auth::getType () const
{
    return _type;
}

int Auth::setType (Type type)
{
    LIBENCLOUD_RETURN_IF (type < FirstType || type > LastType, ~0);
    
    _type = type;

    return 0;
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
        LIBENCLOUD_ERR_IF (1);

    return 0;
err:
    return ~0;
}

const QString &Auth::getUrl () const
{
    return _url;
}

int Auth::setUrl (const QString &url)
{
    LIBENCLOUD_RETURN_IF (!QUrl(url).isValid(), ~0);

    _url = url;

    return 0;
}

const QString &Auth::getUser () const
{
    return _user;
}

int Auth::setUser (const QString &user)
{
    LIBENCLOUD_RETURN_IF (user == "", ~0);

    _user = user;

    return 0;
}

const QString &Auth::getPass () const
{
    return _pass;
}

int Auth::setPass (const QString &pass)
{
    LIBENCLOUD_RETURN_IF (pass == "", ~0);

    _pass = pass;

    return 0;
}

} // namespace libencloud
