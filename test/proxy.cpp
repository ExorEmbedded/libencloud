#include <QTimer>
#include <common/common.h>
#include <common/config.h>
#include <api/api.h>
#include "proxy.h"
#include "test.h"

#define LIBENCLOUD_TEST_PROXY_TYPE  QNetworkProxy::HttpProxy
#define LIBENCLOUD_TEST_PROXY_HOST  "127.0.0.1"
//#define LIBENCLOUD_TEST_PROXY_HOST  "192.168.122.247"
#define LIBENCLOUD_TEST_PROXY_PORT  3128
//#define LIBENCLOUD_TEST_PROXY_PORT  8080
#define LIBENCLOUD_TEST_PROXY_USER  "test"
#define LIBENCLOUD_TEST_PROXY_PASS  "test"
#define LIBENCLOUD_TEST_PROXY_URL   "http://www.google.com"
//#define LIBENCLOUD_TEST_PROXY_URL   LIBENCLOUD_API_URL LIBENCLOUD_API_STATUS_PATH
//#define LIBENCLOUD_TEST_PROXY_SB    "192.168.122.145"
//#define LIBENCLOUD_TEST_PROXY_URL   "https://" LIBENCLOUD_TEST_PROXY_SB "/manage/status/status.access.config/"

TestProxy::TestProxy ()
{
}

void TestProxy::run ()
{
    LIBENCLOUD_TRACE;

    QUrl url(LIBENCLOUD_TEST_PROXY_URL);
    QUrl params;
    QMap<QByteArray, QByteArray> headers;
    QSslConfiguration conf;

    connect(&_client, SIGNAL(error(libencloud::Error)),
            this, SLOT(_clientError(libencloud::Error)));
    connect(&_client, SIGNAL(complete(QString)),
            this, SLOT(_clientComplete(QString)));

    _proxy.setType(LIBENCLOUD_TEST_PROXY_TYPE);
    _proxy.setHostName(LIBENCLOUD_TEST_PROXY_HOST);
    _proxy.setPort(LIBENCLOUD_TEST_PROXY_PORT);
    _proxy.setUser(LIBENCLOUD_TEST_PROXY_USER);
    _proxy.setPassword(LIBENCLOUD_TEST_PROXY_PASS);

    _proxyFactory = new libencloud::ProxyFactory();
    LIBENCLOUD_ERR_IF (_proxyFactory == NULL);

    _proxyFactory->setInclusive(false);

    // set host names manually only if inclusive == false
    //_proxyFactory->add(LIBENCLOUD_TEST_PROXY_SB);
    //_proxyFactory->add("www.google.com");

    _proxyFactory->setApplicationProxy(_proxy);

    // lib takes ownership of our proxy factory
    QNetworkProxyFactory::setApplicationProxyFactory(_proxyFactory);

    _client.run(url, params, headers, conf);

    qApp->exec();

err:
    return;
}

void TestProxy::_clientError (const libencloud::Error &err)
{
    LIBENCLOUD_DBG("err: " << err.toString());

    qApp->quit();
}

void TestProxy::_clientComplete (const QString &response)
{
    LIBENCLOUD_DBG("response: " << response);;

    qApp->quit();
}
