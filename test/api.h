#ifndef _LIBENCLOUD_TEST_API_H_
#define _LIBENCLOUD_TEST_API_H_

#include <QCoreApplication>
#include <QObject>
#include <QUuid>
#include <encloud/Api/Common>
#include <encloud/Api/Auth>
#include <encloud/Api/Setup>
#include <encloud/Api/Status>

class TestApi : public QObject
{
    Q_OBJECT

public:
    TestApi ();

signals:

#ifdef LIBENCLOUD_MODE_SECE
    void licenseSupply (const QUuid &uuid);
#endif

#ifdef LIBENCLOUD_MODE_QIC
    void portSupply (int);
#endif

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

private:
    libencloud::StatusApi _statusApi;
    libencloud::SetupApi _setupApi;
};

#endif
