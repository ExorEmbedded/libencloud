#ifndef _LIBENCLOUD_PRIV_MESSAGE_H_
#define _LIBENCLOUD_PRIV_MESSAGE_H_

#include <QString>
#include <QtPlugin>
#include <QVariant>
#include <encloud/Client>
#include <encloud/Error>
#include <common/config.h>

namespace libencloud {

/**
 * [process] -> encode() -> client.run() -> decode() -> [processed]
 */
class MessageInterface
{
public:
    int clear ();
    int setConfig (Config *cfg);
    int setClient (Client *client);
    int setData (const QVariant &data);

signals:
    // an error occurred
    virtual void error (const Error &error) = 0;

    // message processing COMPLETED ()
    virtual void processed () = 0;

public slots:
    // START processing the message (pack before sending)
    virtual int process () = 0;

protected:
    Config *_cfg;
    Client *_client;
    QVariant _data;
};

} // namespace libencloud

Q_DECLARE_INTERFACE(libencloud::MessageInterface, "com.endian.libencloud.MessageInterface/0.1")

#endif  /* _LIBENCLOUD_PRIV_MESSAGE_H_ */
