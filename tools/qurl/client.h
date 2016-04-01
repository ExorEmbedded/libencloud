#ifndef _LIBENCLOUD_QURL_CLIENT_H_
#define _LIBENCLOUD_QURL_CLIENT_H_

#include <encloud/Client>

namespace qurl {

class Client : public QObject
{
    Q_OBJECT

public:
    Client ();
    bool run (const QString &url, const QVariantMap &opts);

private slots:
    void clientError (const libencloud::Error &err);
    void clientComplete (const QString &response);

private:
    libencloud::Client client;
};

} // namespace qurl

#endif //_LIBENCLOUD_QURL_CLIENT_H_
