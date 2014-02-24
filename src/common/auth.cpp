#include <encloud/Auth>

namespace libencloud {

//
// public methods
//

Auth::Auth ()
    : _valid(false)
{
}

Auth::Auth (const QString &type, const QString &url, 
        const QString &user, const QString &pass)
    : _valid(false)
    , _type(type)
    , _url(url)
    , _user(user)
    , _pass(pass)
{
    // type and url are optional

    if (user == "" || pass == "")
        goto err;

    _valid = true;

err:
    return; 
}

bool Auth::isValid () const
{
    return _valid;
}

const QString &Auth::getType () const
{
    return _type;
}

void Auth::setType (const QString &type)
{
    _type = type;
}

const QString &Auth::getUrl () const
{
    return _url;
}

void Auth::setUrl (const QString &url)
{
    _url = url;
}

const QString &Auth::getUser () const
{
    return _user;
}

void Auth::setUser (const QString &user)
{
    _user = user;
}

const QString &Auth::getPass () const
{
    return _pass;
}

void Auth::setPass (const QString &pass)
{
    _pass = pass;
}

} // namespace libencloud
