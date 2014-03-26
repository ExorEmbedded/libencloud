#ifndef _LIBENCLOUD_PRIV_SETUP_ECE_RETRCERTMSG_H_
#define _LIBENCLOUD_PRIV_SETUP_ECE_RETRCERTMSG_H_

#include <QUuid>
#include <QSslCertificate>
#include <QString>
#include <common/message.h>

namespace libencloud {

class RetrCertMsg : public QObject, public MessageInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::MessageInterface)

signals:
    void error (const libencloud::Error &err);
    void processed ();

public slots:
    int process ();

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
    QString _csr;

    //response outputs
    QDateTime _time;
    QSslCertificate _cert;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_ECE_RETRCERTMSG_H_ */
