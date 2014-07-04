#ifndef _LIBENCLOUD_TEST_UTILS_H_
#define _LIBENCLOUD_TEST_UTILS_H_

#include <QObject>

class TestUtils : public QObject
{
    Q_OBJECT

private slots:
    void run ();

private:
    void testVariantMerge (const QString &to, const QString &from, const QString &sout);
};

#endif
