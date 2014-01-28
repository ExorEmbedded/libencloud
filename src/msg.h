#ifndef _ENCLOUD_MSG_H_
#define _ENCLOUD_MSG_H_

#include <QDebug>
#include <QString>
#include <QDateTime>
#include <QSslCertificate>
#include <QUrl>
#include <QUuid>
#include <encloud.h>
#include "helpers.h"

namespace encloud {

/** 
 * \brief Simple request-response message objects for HTTP communication.
 * 
 * 1) create a specific child of Message object
 * 2) set public input fields
 * 3) build parameters via encodeRequest() - generally JSON-encoded
 * 4) decoded response strings via decodeResponse()
 * 5) access public output fields
 */

class Message
{
public:
    Message ();
    virtual encloud_rc_t encodeRequest (QUrl &url, QUrl &params) = 0;
    virtual encloud_rc_t decodeResponse (QString &response, QString &errString) = 0;

    //response outputs
    QDateTime time;
};

class MessageRetrInfo : public Message 
{
public:
    MessageRetrInfo ();
    encloud_rc_t encodeRequest (QUrl &url, QUrl &params);
    encloud_rc_t decodeResponse (QString &response, QString &errString);

    //request inputs
    QUuid license;
    QString hwInfo;

    //response outputs
    bool valid;
    QDateTime expiry;
    QVariant csrTmpl;
    QSslCertificate caCert;
};

class MessageRetrCert : public Message 
{
public:
    MessageRetrCert ();
    encloud_rc_t encodeRequest (QUrl &url, QUrl &params);
    encloud_rc_t decodeResponse (QString &response, QString &errString);

    //request inputs
    QUuid license;
    QString hwInfo;
    QString csr;

    //response outputs
    QSslCertificate cert;
};

class MessageRetrConf : public Message 
{
public:
    MessageRetrConf ();
    encloud_rc_t encodeRequest (QUrl &url, QUrl &params);
    encloud_rc_t decodeResponse (QString &response, QString &errString);

    //response outputs
    QString vpnIp;
    int vpnPort;
    QString vpnProto;
    QString vpnType;
};

} // namespace encloud

#endif
