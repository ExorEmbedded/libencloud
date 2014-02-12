#ifndef _LIBENCLOUD_PRIV_SETUP_ECE_RETRINFOMSG_H_
#define _LIBENCLOUD_PRIV_SETUP_ECE_RETRINFOMSG_H_

#include <QString>
#include <QUuid>
#include <QDateTime>
#include <QSslCertificate>
#include <common/client.h>
#include <common/message.h>

namespace libencloud {

class RetrInfoMsg : public QObject, public MessageInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::MessageInterface)

public:
    int init ();

signals:
    void error ();
    void need (const QString &what);
    void processed ();

public slots:
    void process ();

private slots:
    void _clientComplete (const QString &response);

private:
    int _packRequest ();
    int _encodeRequest (QUrl &url, QUrl &params);
    int _decodeResponse (const QString &response);
    int _unpackResponse ();

    //request inputs
    QUuid _license;
    QString _hwInfo;

    //response outputs
    QDateTime _time;
    bool _valid;
    QDateTime _expiry;
    QVariant _csrTmpl;
    QSslCertificate _caCert;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_ECE_RETRINFOMSG_H_ */
