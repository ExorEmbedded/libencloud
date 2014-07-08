#ifndef _LIBENCLOUD_TEST_SEC_H_
#define _LIBENCLOUD_TEST_SEC_H_

#include <QCoreApplication>
#include <QObject>
#include <encloud/Client>

class TestSec : public QObject
{
    Q_OBJECT

public:
    TestSec ();

private slots:
    void run ();
    void _testClient();
    void _clientError (const libencloud::Error &err);
    void _clientComplete (const QString &response);

private:
    libencloud::Client *_client;
};

#endif
