#ifndef _LIBENCLOUD_PRIV_SETUP_ECE_RETRCONFMSG_H_
#define _LIBENCLOUD_PRIV_SETUP_ECE_RETRCONFMSG_H_

#include <QDateTime>
#include <QString>
#include <common/message.h>

namespace libencloud {

class RetrConfMsg : public QObject, public MessageInterface
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

    //response outputs
    QDateTime _time;
    QString _vpnIp;
    int _vpnPort;
    QString _vpnProto;
    QString _vpnType;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_ECE_RETRCONFMSG_H_ */
