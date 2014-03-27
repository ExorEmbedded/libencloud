#include <encloud/Error>
#include <common/common.h>
#include <common/config.h>

namespace libencloud 
{

//
// public members
//

int Error::__seq = 0;

Error::Error ()
    : _isValid(false)
    , _code(CodeUndefined)
    , _seq(-1)
{
}

Error::Error (Code code, QString extra)
    : _isValid(false)
    , _code(CodeUndefined)
    , _seq(++__seq)
{
    LIBENCLOUD_ERR_IF (setCode(code));
    _desc = _code2Desc(code);
    LIBENCLOUD_ERR_IF (setExtra(extra));

    LIBENCLOUD_DBG(toString());

    _isValid = true;

err:
    return;
}

Error::Error (const QString &msg)
    : _isValid(false)
    , _code(CodeUndefined)
    , _seq(++__seq)
{
    LIBENCLOUD_DBG(msg);

    _desc = msg;

    _isValid = true;
}

bool Error::isValid () const   { return _isValid; }

bool Error::operator == (const Error &e) const
{
    return (_code == e._code &&
            _seq == e._seq);
}

bool Error::operator != (const Error &e) const
{
    return !(e == *this);
}

QString Error::toString () const
{
    QString str;

    str += QString::number(_code) + ".";
    str += QString::number(_seq);
       
    if (_desc != "")
        str += ": " + _desc;

    if (_extra != "")
        str += " - " + _extra;

    return str;
}

Error::Code Error::getCode () const
{
    return _code;
}

int Error::setCode (Error::Code code)
{
    if (code < CodeFirst || code > CodeLast)
        goto err;

    _code = code;

    return 0;
err:
    return ~0;
}

int Error::getSeq () const
{
    return _seq;
}

int Error::setSeq (int seq)
{
    LIBENCLOUD_ERR_IF (seq < 0);

    _seq = seq;

    return 0;
err:
    return ~0;
}

QString Error::getDesc () const
{
    return _desc;
}

int Error::setDesc (const QString &desc)
{
    _desc = desc;

    return 0;
}

QString Error::getExtra () const
{
    return _extra;
}

int Error::setExtra (const QString &extra)
{
    _extra = extra;

    return 0;
}

//
// private members
//

QString Error::_code2Desc (Code code)
{
    switch (code)
    {
        case CodeUndefined:
            return QObject::tr("<Undefined>");

        // 0xx
        case CodeSuccess:
            return QObject::tr("Operation Successful");
        case CodeServerUnreach:
            return QObject::tr("Server Unreachable. Please check address and connectivity.");

        // 1xx
        case CodeServiceUnreach:
            return QObject::tr("Local Encloud Service Unreachable");

        // 5xx
        case CodeAuthFailed:
            return QObject::tr("Server Authentication Failure. Please check credentials.");

        // 6xx
        case CodeProxyAuthFailed:
            return QObject::tr("Proxy Authentication Failure. Please check credentials.");

        // 1xxx
        case CodeServerError:
            return QObject::tr("Generic Server Error");
    }

    return "";
}

}  // namespace libencloud
