#ifndef _LIBENCLOUD_PRIV_SETUP_ECE_RETRINFOMSG_H_
#define _LIBENCLOUD_PRIV_SETUP_ECE_RETRINFOMSG_H_

#include <QDateTime>
#include <QSslCertificate>
#include <QString>
#include <QUuid>
#include <encloud/Client>
#include <common/message.h>

namespace libencloud {

class RetrInfoMsg : public QObject, public MessageInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::MessageInterface)

signals:
    void error (const libencloud::Error &err);
    void need (const QString &what);
    void processed ();

public slots:
    int process ();

#ifdef LIBENCLOUD_MODE_SECE
    void licenseReceived (const QUuid &uuid);
#endif

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
