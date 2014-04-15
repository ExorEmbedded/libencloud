#include <encloud/Proxy>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

QString Proxy::toString (const QNetworkProxy &p)
{
    QString s;
    
    s += "type: " + QString::number(p.type()) + ", ";
    s += "host: " + p.hostName() + ", ";
    s += "port: " + QString::number(p.port()) + ", ";
    s += "user: " + p.user() + ", ";
    s += "pass: <not shown>";

    return s;
}

ProxyFactory::ProxyFactory ()
    : _inclusive(true)
{
    LIBENCLOUD_TRACE;
}

/**
 * inclusive=true       : add hosts to be proxied
 * inclusive=false      : add hosts to be blacklisted from proxy
 */
int ProxyFactory::setInclusive (bool inclusive)
{
    LIBENCLOUD_DBG("inclusive: " << inclusive);

    if (!inclusive)
    {
        // make sure local requests aren't proxied
        _hosts.clear();
        _hosts.append("127.0.0.1");
        _hosts.append("localhost");
    }

    _inclusive = inclusive;

    return 0;
}

int ProxyFactory::setApplicationProxy (const QNetworkProxy &proxy)
{
    LIBENCLOUD_DBG(Proxy::toString(proxy));

    _proxy = proxy;

    return 0;
}

int ProxyFactory::add (const QString &host)
{
    LIBENCLOUD_DBG("host: " << host);

    if (_hosts.contains(host))
        return 0;

    _hosts.append(host);

    return 0;
}

int ProxyFactory::remove (const QString &host)
{
    LIBENCLOUD_DBG("host: " << host);

    _hosts.removeAll(host);

    return 0;
}

QList<QNetworkProxy> ProxyFactory::queryProxy (const QNetworkProxyQuery &query)
{
    QList<QNetworkProxy> proxies;
    QString host = query.peerHostName();
    QString log;

    if (!query.url().isValid())
    {
        proxies << QNetworkProxy(QNetworkProxy::NoProxy);
        return proxies;
    }

    log = "url: " + query.url().toString();

    if ((_inclusive && _hosts.contains(host)) || 
            (!_inclusive && !_hosts.contains(host)))
    {
        log += ", proxy: " + Proxy::toString(_proxy);
        proxies << _proxy;
    }
    else
    {
        log += ", NO proxy";
        proxies << QNetworkProxy(QNetworkProxy::NoProxy);
    }

    if (!_isLocal(host))
        LIBENCLOUD_DBG(log);

    return proxies;
}

//
// private methods
//

bool ProxyFactory::_isLocal (const QString &host)
{
    return (host == "localhost" ||
            host == "127.0.0.1");
}

} // namespace libencloud
