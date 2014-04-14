#ifndef _LIBENCLOUD_TEST_JSON_H_
#define _LIBENCLOUD_TEST_JSON_H_

#include <QString>
#include <encloud/Json>

class TestJson : public QObject
{
    Q_OBJECT

private slots:
    void run ();

private:
    void test (const QString &s, bool jsonp);
};

#endif
