#ifndef _LIBENCLOUD_PRIV_MESSAGE_H_
#define _LIBENCLOUD_PRIV_MESSAGE_H_

#include <QtPlugin>
#include <common/config.h>
#include <common/client.h>

namespace libencloud {

/**
 * [process] -> encode() -> client.run() -> decode() -> [processed]
 */
class MessageInterface
{
public:
    void setConfig (Config *cfg);
    void setClient (Client *client);
    virtual int init () = 0;

signals:
    // an error occurred
    virtual void error () = 0;

    // message processing COMPLETED ()
    virtual void processed () = 0;

public slots:
    // START processing the message (pack before sending)
    virtual void process () = 0;

protected:
    Config *_cfg;
    Client *_client;
};

} // namespace libencloud

Q_DECLARE_INTERFACE(libencloud::MessageInterface, "com.endian.libencloud.MessageInterface/0.1")

#endif  /* _LIBENCLOUD_PRIV_MESSAGE_H_ */
