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

int ProxyFactory::setApplicationProxy (const QNetworkProxy &proxy)
{
    LIBENCLOUD_DBG(Proxy::toString(proxy));

    _proxy = proxy;

    return 0;
}

QList<QNetworkProxy> ProxyFactory::queryProxy (const QNetworkProxyQuery &query)
{
    QList<QNetworkProxy> proxies;
    QString host = query.peerHostName();

//    LIBENCLOUD_DBG("url: " << query.url().toString());

    if (host == "127.0.0.1" || host == "localhost")
        proxies << QNetworkProxy(QNetworkProxy::NoProxy);
    else
        proxies << _proxy;

    return proxies;
}

} // namespace libencloud
