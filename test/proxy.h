#ifndef _LIBENCLOUD_TEST_PROXY_H_
#define _LIBENCLOUD_TEST_PROXY_H_

#include <QCoreApplication>
#include <QObject>
#include <encloud/Proxy>
#include <encloud/Client>
#include <encloud/Server>

class TestProxy : public QObject
{
    Q_OBJECT

public:
    TestProxy ();

private slots:
    void run ();
    void _clientError (const libencloud::Error &err);
    void _clientComplete (const QString &response);

private:
    QNetworkProxy _proxy;
    libencloud::ProxyFactory *_proxyFactory;
    libencloud::Client _client; 
};

#endif

