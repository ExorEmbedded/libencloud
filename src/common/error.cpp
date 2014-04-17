#include <QtCore/qmath.h>
#include <QTimer>
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
    , _seq(++__seq)
{
    _desc = _code2Desc(_code);
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
    _seq = seq;

    return 0;
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
        // emitted when no specific error code is given
        case CodeUndefined:
            return QObject::tr("Generic Error");

        // 0xx
        case CodeSuccess:
            return QObject::tr("Operation Successful");
        case CodeSystemError:
            return QObject::tr("System Error - please check resource status");

        // 1xx
        case CodeServiceUnreach:
            return QObject::tr("Local Encloud Service Unreachable");

        // 2xx
        case CodeServerUnreach:
            return QObject::tr("Server Unreachable");

        // 3xx
        case CodeAuthFailed:
            return QObject::tr("Server Authentication Failure - please check credentials");

        // 5xx
        case CodeProxyAuthFailed:
            return QObject::tr("Proxy Authentication Failure - please check credentials");

        // 1xxx
        case CodeServerError:
            return QObject::tr("Generic Server Error");
    }

    return "";
}

void Retry::setTimeout (int timeout)
{
    LIBENCLOUD_ERR_IF (timeout <= 0);

    _timeout = timeout;
err:
    return;
}

void Retry::setBackoff (int base, int max)
{
    LIBENCLOUD_ERR_IF (base <= 1);
    LIBENCLOUD_ERR_IF (max < 1);

    _base = base;
    _max = max;
err:
    return;
}

void Retry::schedule (QObject *receiver, const char *member)
{
    int secs;

    LIBENCLOUD_ERR_IF (receiver == NULL);
    LIBENCLOUD_ERR_IF (member == NULL);

    if (_base > 0)  // exponential backoff
    {
        secs = qMin(_max, (int) qPow(_base, _backoff));
        if (secs < _max)
            _backoff++;
    }
    else  // fixed
    {
        secs = _timeout;
    }

    LIBENCLOUD_DBG("retrying in " << QString::number(secs) << " seconds");

    QTimer::singleShot(secs * 1000, receiver, member);

err:
    return;
}

void Retry::reset ()
{
    _backoff = 1;
}


}  // namespace libencloud
