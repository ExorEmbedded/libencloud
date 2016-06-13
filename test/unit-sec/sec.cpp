#include <QTimer>
#include <encloud/Common>
#include <encloud/Error>
#include <common/common.h>
#include <common/config.h>
#include "sec.h"

#define TEST_URL        "https://192.168.122.247/some/invalid/url"
#define TEST_CA_CERT    "/tmp/ca.pem"
#define TEST_P12        "/tmp/test.p12"

TestSec::TestSec ()
    : _client(NULL)
{
    LIBENCLOUD_TRACE;
}

void TestSec::run ()
{
    LIBENCLOUD_TRACE;

    // run this test for 5s
    QTimer::singleShot(5000, qApp, SLOT(quit()));

    _testClient();
    QTimer::singleShot(1000, this, SLOT(_testClient()));
    QTimer::singleShot(2000, this, SLOT(_testClient()));
    QTimer::singleShot(3000, this, SLOT(_testClient()));

    qApp->exec();
}

void TestSec::_testClient ()
{
    static int i = 0;
    QMap<QByteArray, QByteArray> headers;
    QList<QSslCertificate> cas(QSslCertificate::fromPath(TEST_CA_CERT));
    QSslConfiguration sslconf;

    LIBENCLOUD_DELETE(_client);
    LIBENCLOUD_ERR_IF ((_client = new libencloud::Client) == NULL);

    connect(_client, SIGNAL(error(libencloud::Error)), this, SLOT(_clientError(libencloud::Error)));
    connect(_client, SIGNAL(complete(QString, QMap<QByteArray, QByteArray>)), this, SLOT(_clientComplete(QString)));

    LIBENCLOUD_DBG("i = " << i);
    LIBENCLOUD_DBG("number of cas: " << sslconf.caCertificates().count());

    // only set CA certificate on even requests
    if ((i++ % 2) == 0)
    {
        LIBENCLOUD_DBG("*** Setting CA cert ***");
        sslconf.setCaCertificates(cas);
    }

    _client->get(QUrl(TEST_URL), headers, sslconf);

err:
    return;
}

void TestSec::_clientError (const libencloud::Error &err)
{
    LIBENCLOUD_DBG("error: " << err.toString());
}

void TestSec::_clientComplete (const QString &response)
{
    LIBENCLOUD_DBG("response: " << response);
}
