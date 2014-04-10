#include <encloud/Proxy>
#include <common/common.h>
#include <common/config.h>

namespace libencloud {

int ProxyFactory::setApplicationProxy (const QNetworkProxy &proxy)
{
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
