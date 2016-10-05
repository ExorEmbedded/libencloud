#ifndef _LIBENCLOUD_PRIV_SETUP_QCC_CLOSEMSG_H_
#define _LIBENCLOUD_PRIV_SETUP_QCC_CLOSEMSG_H_

#include <QNetworkAccessManager>
#include <common/message.h>

#define LIBENCLOUD_SETUP_QCC_CLOSE_URL        "/manage/commands/commands.access.client.closeVPNConnection/"

namespace libencloud {

class CloseMsg : public QObject, public MessageInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::MessageInterface)

public:
    CloseMsg();
    int clear ();
    int setNetworkAccessManager (QNetworkAccessManager *qnam);

signals:
    void error (const libencloud::Error &err);
    void processed ();

public slots:
    int process ();
    void authSupplied (const Auth &auth);

private slots:
    void _error (const libencloud::Error &error);
    void _clientComplete (const QString &response, const QMap<QByteArray, QByteArray> &headers);

private:
    int _packRequest ();
    int _encodeRequest (QUrl &url, QUrl &params);
    int _decodeResponse (const QString &response, const QMap<QByteArray, QByteArray> &headers);

    //request inputs
    Auth _sbAuth;
    QNetworkAccessManager *_qnam;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_QCC_CLOSEMSG_H_ */
