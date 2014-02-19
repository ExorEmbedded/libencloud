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
    _valid = true;
}

bool Auth::isValid () const
{
    return _valid;
}

const QString &Auth::getType () const
{
    return _type;
}

const QString &Auth::getUrl () const
{
    return _url;
}

const QString &Auth::getUser () const
{
    return _user;
}

const QString &Auth::getPass () const
{
    return _pass;
}

} // namespace libencloud
