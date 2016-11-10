#include <QTimer>
#include <encloud/Common>
#include <common/common.h>
#include <common/config.h>
#include "api.h"
#include "test.h"

TestApi::TestApi ()
{
    //
    // Status Api
    //

    connect(&_statusApi, SIGNAL(apiState(libencloud::State)),
            this, SLOT(_statusApiState(libencloud::State)));
    connect(&_statusApi, SIGNAL(apiError(libencloud::Error)),
            this, SLOT(_statusApiError(libencloud::Error)));
    connect(&_statusApi, SIGNAL(apiProgress(libencloud::Progress)),
            this, SLOT(_statusApiProgress(libencloud::Progress)));
    connect(&_statusApi, SIGNAL(apiNeed(QString, QVariant)),
            this, SLOT(_statusApiNeed(QString)));

    //
    // Setup API
    //

#ifdef LIBENCLOUD_MODE_ECE
    TEST_MSG("testing ECE API");
    connect(&_setupApi, SIGNAL(poiReceived(libencloud::Api::ResultCode, QUuid)),
                this, SLOT(_poiReceived(libencloud::Api::ResultCode, QUuid)));
#endif

#ifdef LIBENCLOUD_MODE_SECE
    TEST_MSG("testing SECE API");
    connect(this, SIGNAL(licenseSupply(QUuid)),
                &_setupApi, SLOT(licenseSupply(QUuid)));
    connect(&_setupApi, SIGNAL(licenseSent(libencloud::Api::ResultCode)),
                this, SLOT(_licenseSent(libencloud::Api::ResultCode)));
#endif

#ifdef LIBENCLOUD_MODE_QCC
    TEST_MSG("testing QCC API");
    connect(this, SIGNAL(portSupply(int)),
                &_setupApi, SLOT(portSupply(int)));
    connect(&_setupApi, SIGNAL(portSent(libencloud::Api::ResultCode)),
                this, SLOT(_portSent(libencloud::Api::ResultCode)));
#endif

    TEST_MSG("testing Cloud API");
    connect(this, SIGNAL(actionRequest(QString, libencloud::Params)),
                &_cloudApi, SLOT(actionRequest(QString, libencloud::Params)));
    connect(&_cloudApi, SIGNAL(actionSent(libencloud::Api::ResultCode)),
                this, SLOT(_actionSent(libencloud::Api::ResultCode)));

    TEST_MSG("testing Config API");
    connect(this, SIGNAL(configSupply(QVariant)),
                &_configApi, SLOT(configSupply(QVariant)));
    connect(&_configApi, SIGNAL(configSent(libencloud::Api::ResultCode)),
                this, SLOT(_configSent(libencloud::Api::ResultCode)));
}

void TestApi::run ()
{
    LIBENCLOUD_TRACE;

    // run this test for 5s
    QTimer::singleShot(5000, qApp, SLOT(quit()));

    LIBENCLOUD_DBG("Starting Status API");

    // default port assumed (don't wait for settings)
    _statusApi.start(1000);

    LIBENCLOUD_DBG("Testing Setup API");

#ifdef LIBENCLOUD_MODE_ECE
    _setupApi.poiRetrieve();
#endif

#ifdef LIBENCLOUD_MODE_SECE
    //emit licenseSupply(QUuid("AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE"));
    emit licenseSupply(QUuid("94C97E4B-AB8C-4DD6-B06B-EF3E18ED2D83"));
#endif

#ifdef LIBENCLOUD_MODE_QCC
    emit portSupply(12345);
#endif

    LIBENCLOUD_DBG("Testing Cloud API");

    libencloud::Params params;
    params.append(libencloud::Param("myKey1", "myVal1"));
    params.append(libencloud::Param("myKey2", "myVal2"));

    emit actionRequest("myAction", params);

    _testConfig();

    qApp->exec();
}

void TestApi::_statusApiState (libencloud::State st)
{
    LIBENCLOUD_DBG("state: " << QString::number(st));
}

void TestApi::_statusApiError (const libencloud::Error &err)
{
    LIBENCLOUD_DBG("err: " << err.toString());
}

void TestApi::_statusApiProgress (const libencloud::Progress &progress)
{
    LIBENCLOUD_DBG("step: " << QString::number(progress.getStep()));
}

void TestApi::_statusApiNeed (const QString &need)
{
    LIBENCLOUD_DBG("need: " << need);
}

#ifdef LIBENCLOUD_MODE_ECE
void TestApi::_poiReceived (libencloud::Api::ResultCode rc, QUuid uuid)
{
    QVERIFY (rc == libencloud::Api::SuccessRc);

    LIBENCLOUD_DBG("uuid: " << uuid.toString());
}
#endif

#ifdef LIBENCLOUD_MODE_SECE
void TestApi::_licenseSent (libencloud::Api::ResultCode rc)
{
    QVERIFY (rc == libencloud::Api::SuccessRc);

    LIBENCLOUD_DBG("rc: " << QString::number(rc));
}
#endif

#ifdef LIBENCLOUD_MODE_QCC
void TestApi::_portSent (libencloud::Api::ResultCode rc)
{
    QVERIFY (rc == libencloud::Api::SuccessRc);

    LIBENCLOUD_DBG("rc: " << QString::number(rc));
}
#endif

void TestApi::_actionSent (libencloud::Api::ResultCode rc)
{
    QVERIFY (rc == libencloud::Api::SuccessRc);

    LIBENCLOUD_DBG("rc: " << QString::number(rc));
}

void TestApi::_configSent (libencloud::Api::ResultCode rc)
{
    QVERIFY (rc == libencloud::Api::SuccessRc);

    LIBENCLOUD_DBG("rc: " << QString::number(rc));
}

void TestApi::_testConfig ()
{
    LIBENCLOUD_DBG("Testing Config API");

    QVariantMap configKey;
    QVariantMap configSubKey;
    QVariantMap sslSubKey;

    configKey["timeout"] = 300;

    configSubKey["lev"] = 7;
    configKey["log"] = configSubKey;

    sslSubKey["verify_ca"] = "false";
    configKey["ssl"] = sslSubKey;

    emit configSupply(configKey);

    LIBENCLOUD_DBG("clearing config");

    emit configSupply(QVariantMap());

    return;
}
