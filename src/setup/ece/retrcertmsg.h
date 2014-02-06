#ifndef _LIBENCLOUD_PRIV_SETUP_ECE_RETRCERTMSG_H_
#define _LIBENCLOUD_PRIV_SETUP_ECE_RETRCERTMSG_H_

#include <QString>
#include <QUuid>
#include <QSslCertificate>
#include <QUuid>
#include <common/message.h>

namespace libencloud {

class RetrCertMsg : public QObject, public MessageInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::MessageInterface)

public:
    int init ();

signals:
    void error ();
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
    QString _csr;

    //response outputs
    QDateTime _time;
    QSslCertificate _cert;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_ECE_RETRCERTMSG_H_ */
