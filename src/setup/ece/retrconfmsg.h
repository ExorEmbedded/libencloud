#ifndef _LIBENCLOUD_PRIV_SETUP_ECE_RETRCONFMSG_H_
#define _LIBENCLOUD_PRIV_SETUP_ECE_RETRCONFMSG_H_

#include <QDateTime>
#include <QString>
#include <common/message.h>
#include <common/vpn/vpnconfig.h>

namespace libencloud {

class RetrConfMsg : public QObject, public MessageInterface
{
    Q_OBJECT
    Q_INTERFACES (libencloud::MessageInterface)

public:
    const VpnConfig *getVpnConfig ();

signals:
    void error (QString msg = "");
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

    //response outputs
    QDateTime _time;
    VpnConfig _vpnConfig;
};

} // namespace libencloud

#endif  /* _LIBENCLOUD_PRIV_SETUP_ECE_RETRCONFMSG_H_ */
