#ifndef _LIBENCLOUD_TEST_API_H_
#define _LIBENCLOUD_TEST_API_H_

#include <QCoreApplication>
#include <QObject>
#include <QUuid>
#include <encloud/Api/CommonApi>
#include <encloud/Api/AuthApi>
#include <encloud/Api/CloudApi>
#include <encloud/Api/SetupApi>
#include <encloud/Api/StatusApi>

class TestApi : public QObject
{
    Q_OBJECT

public:
    TestApi ();

signals:

    //
    // Setup Api
    //

#ifdef LIBENCLOUD_MODE_SECE
    void licenseSupply (const QUuid &uuid);
#endif

#ifdef LIBENCLOUD_MODE_QIC
    void portSupply (int);
#endif

    //
    // Cloud Api
    //

    void actionRequest (const QString &action, const libencloud::Params &params);

private slots:
    void run();

    //
    // Status Api
    //

    void _statusApiState (libencloud::State st);
    void _statusApiError (const QString &msg);
    void _statusApiProgress (const libencloud::Progress &progress);
    void _statusApiNeed (const QString &need);

    //
    // Setup Api
    //

#ifdef LIBENCLOUD_MODE_ECE
    void _poiReceived (libencloud::Api::ResultCode rc, QUuid uuid);
#endif

#ifdef LIBENCLOUD_MODE_SECE
    void _licenseSent (libencloud::Api::ResultCode rc);
#endif

#ifdef LIBENCLOUD_MODE_QIC
    void _portSent (libencloud::Api::ResultCode rc);
#endif

    void _actionSent (libencloud::Api::ResultCode rc);

private:
    libencloud::StatusApi _statusApi;
    libencloud::SetupApi _setupApi;
    libencloud::CloudApi _cloudApi;
};

#endif
