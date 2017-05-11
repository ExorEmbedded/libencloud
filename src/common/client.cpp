#include <QTimer>
#include <encloud/Client>
#include <common/common.h>
#include <common/config.h>

#define EMIT_ERROR(msg) LIBENCLOUD_EMIT(error(Error(msg)))
#define EMIT_ERROR_ERR_IF(cond, msg) LIBENCLOUD_EMIT_ERR_IF(cond, error(Error(msg)))

// Allow debug deactivation via API
#define CLIENT_DBG(msg) do { \
    if (_debug) \
        LIBENCLOUD_DBG(msg); \
    } while (0);

namespace libencloud {

quint32 Client::_id = 0;

Client::Client ()
    : _qnam(NULL)
    , _qnamExternal(false)
    , _verifyCA(true)
    , _debug(true)
    , _timeout(LIBENCLOUD_CLIENT_TIMEOUT)
    , _timeoutRetry(false)
    , _sslError(false)
{
    LIBENCLOUD_TRACE;

    _qnam = new QNetworkAccessManager;
    LIBENCLOUD_ERR_IF (_qnam == NULL);

    _connectQnam();

err:
    return;
}

Client::~Client ()
{
    LIBENCLOUD_TRACE;

    qDeleteAll(_conns);
    _conns.clear();

    if (!_qnamExternal)
        LIBENCLOUD_DELETE(_qnam);
}

QNetworkAccessManager *Client::getNetworkAccessManager ()
{
    return _qnam;
}

int Client::setNetworkAccessManager (QNetworkAccessManager *qnam)
{
    LIBENCLOUD_ERR_IF (qnam == NULL);
    LIBENCLOUD_ERR_IF (_qnamExternal);  // has already been set externally

    LIBENCLOUD_DELETE (_qnam);
    
    _qnam = qnam;
    _qnamExternal = true;

    _connectQnam();

    return 0;
err:
    return ~0;
}

void Client::setVerifyCA (bool b) 
{
    LIBENCLOUD_DBG("[Client] verify CA: " << b);

    _verifyCA = b;
}

void Client::setDebug (bool b) 
{
    _debug = b;
}

// if timeout is <= 0, it is disabled
void Client::setTimeout (int timeout, bool retry)
{
    _timeout = timeout;
    _timeoutRetry = retry;
}

void Client::reset ()
{
    LIBENCLOUD_TRACE;

    // also stops timers associated with connections
    qDeleteAll(_conns);
    _conns.clear();

    if (_qnam)
        disconnect(_qnam, NULL, this, NULL);

    if (!_qnamExternal)
    {
        if (_qnam)
            _qnam->deleteLater();

        _qnam = new QNetworkAccessManager;
        LIBENCLOUD_ERR_IF (_qnam == NULL);
    }

    _connectQnam();
err:
    return;
}

void Client::run (const QUrl &url, const QUrl &params,
		const QMap<QByteArray, QByteArray> &headers, const QSslConfiguration &conf)
{
	_send(MSG_TYPE_NONE, url, headers, params.encodedQuery(), conf);
}

void Client::get (const QUrl &url, const QMap<QByteArray, QByteArray> &headers,
        const QSslConfiguration &conf)
{
	_send(MSG_TYPE_GET, url, headers, "", conf);
}

void Client::post (const QUrl &url, const QMap<QByteArray, QByteArray> &headers,
        const QByteArray &data, const QSslConfiguration &conf)
{
	_send(MSG_TYPE_POST, url, headers, data, conf);
}

int Client::_sendRequest (MsgType msgType, const QNetworkRequest &request, const QByteArray &data)
{
    Connection *conn = NULL;
    QNetworkReply *reply = NULL;
    _sslError = false;
    _response = "";

    if ((msgType == MSG_TYPE_GET) ||
            (msgType == MSG_TYPE_NONE && data.isEmpty()))
    {
        EMIT_ERROR_ERR_IF ((reply = _qnam->get(request)) == NULL,
                tr("Client failed creating GET request"));
    }
    else if ((msgType == MSG_TYPE_POST) ||
            (msgType == MSG_TYPE_NONE && !data.isEmpty()))
    {
        EMIT_ERROR_ERR_IF ((reply = _qnam->post(request, data)) == NULL,
                tr("Client falied creating POST request"));
    } else {
        LIBENCLOUD_ERR ("bad msgType: " << QString::number(msgType));
    }

    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), 
            this, SLOT(_networkError(QNetworkReply::NetworkError)));

    conn = new Connection(this, reply, _timeout, _timeoutRetry);
    LIBENCLOUD_ERR_IF (conn == NULL);

    connect(conn, SIGNAL(error(libencloud::Error)), 
            this, SIGNAL(error(libencloud::Error)));

    conn->msgType = msgType;
    conn->request = request;
    conn->data = data;

    _conns[reply] = conn;
    return 0;
err:
    LIBENCLOUD_DELETE(conn);
    return ~0;
}

void Client::_send (MsgType msgType, const QUrl &url, const QMap<QByteArray, QByteArray> &headers,
		const QByteArray &data, const QSslConfiguration &conf)
{
    CLIENT_DBG("[Client] id: " << QString::number(++_id) << " to: " << url.toString());
    //CLIENT_DBG(" ### >>>>> ### " << data);

    QNetworkRequest request(url);

#ifndef Q_OS_WINCE
    if (url.scheme() == LIBENCLOUD_SCHEME_HTTPS)
    {
        QSslConfiguration sslConf(conf);

        if (sslConf.caCertificates().count() == 0)
        {
            sslConf.setCaCertificates(QSslConfiguration::defaultConfiguration().caCertificates());
            LIBENCLOUD_DBG(QString("[Client] Loaded %1 System CA Certificates").arg(sslConf.caCertificates().count()))
        }

        /*
        Q_FOREACH(QSslCertificate cert, sslConf.caCertificates())
        {
            LIBENCLOUD_DBG("[Client] CA Cert issuer: " << cert.issuerInfo(QSslCertificate::CommonName));
        }
        */

        request.setSslConfiguration(sslConf);
    }
#endif

    // default headers
    request.setRawHeader("User-Agent", LIBENCLOUD_USERAGENT);
    request.setRawHeader("Host", url.host().toAscii());
    if ((msgType == MSG_TYPE_POST) ||
            (msgType == MSG_TYPE_NONE && !data.isEmpty()))
        request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    // override with passed custom headers
    for (QMap<QByteArray, QByteArray>::const_iterator mi = headers.begin(); mi != headers.end(); mi++)
        request.setRawHeader(mi.key(), mi.value());

    _sendRequest(msgType, request, data);
}

//
// private slots
//

void Client::_proxyAuthenticationRequired (const QNetworkProxy &proxy, QAuthenticator *authenticator)
{ 
    LIBENCLOUD_UNUSED(authenticator); 
    LIBENCLOUD_UNUSED(proxy);

    EMIT_ERROR(tr("Proxy Authentication Required"));
}

void Client::_sslErrors (QNetworkReply *reply, const QList<QSslError> &errors) 
{
#ifndef Q_OS_WINCE
    QList<QSslError> ignoreErrors;

    foreach (QSslError err, errors)
    {
        switch (err.error())
        {
            default:
                if (_verifyCA)
                {
                    CLIENT_DBG("[Client] CRITICAL QSslError (" << (int) err.error() << "): " << err.errorString());
                    LIBENCLOUD_EMIT(error(Error(Error::CodeServerVerifyFailed)));
                    _sslError = true;
                }
                else
                    ignoreErrors.append(err);
                break;
#if 0
            CLIENT_DBG("[Client] Peer Cert subj_CN=" << err.certificate().subjectInfo(QSslCertificate::CommonName) << \
                    " issuer_O=" << err.certificate().issuerInfo(QSslCertificate::Organization)); 
#endif
        }
    }

    reply->ignoreSslErrors(ignoreErrors);
#endif
}

void Client::_networkError (QNetworkReply::NetworkError err)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *> (sender());
    QString extraMsg;
    QVariantMap json;
    bool ok;

    // get message from Switchboard
    extraMsg = reply->readAll();
    json = json::parse(extraMsg, ok).toMap();

    // default to errorString() if json error is not present or invalid
    if (extraMsg == "" || json.isEmpty() || !ok || json["error"].isNull())
        extraMsg = reply->errorString();
    else
        extraMsg = json["error"].toString();

    CLIENT_DBG("[Client] error " << err << ": " << extraMsg);

    switch (err)
    {
        case QNetworkReply::UnknownContentError:
        case QNetworkReply::ProtocolFailure:
            LIBENCLOUD_EMIT(error(Error(Error::CodeServerError, extraMsg)));
            break;

        case QNetworkReply::SslHandshakeFailedError:

            // already emitted in _sslErrors()
            if (_sslError)
                break;

        case QNetworkReply::ContentNotFoundError:
            LIBENCLOUD_EMIT(error(Error(Error::CodeServerNotFound, extraMsg)));
            break;
            // else follow through
        default:
            // url is too much detail for end user
            //LIBENCLOUD_EMIT(error(Error(Error::CodeServerUnreach, extraMsg + ", url: " + reply->url().toString())));
            LIBENCLOUD_EMIT(error(Error(Error::CodeServerUnreach, extraMsg)));
            break;
    }
}

/**
 * "Note: After the request has finished, it is the responsibility of the user
 * to delete the QNetworkReply object at an appropriate time. Do not directly
 * delete it inside the slot connected to finished()"
 */
void Client::_finished (QNetworkReply *reply) 
{
    LIBENCLOUD_RETURN_IF (reply == NULL, );

    QList<QNetworkReply::RawHeaderPair> headerPairs = reply->rawHeaderPairs();
    QMap<QByteArray, QByteArray> headers;
    Connection *conn = _conns[reply];
    LIBENCLOUD_RETURN_IF (conn == NULL, );
    conn->stop();

    // Possible error code remappings (if required because they should not
    // occur with proper configuration):
    //  - handshake failed (6)
    //  - key values mismatch (99)
    if (reply->error())
    {
        LIBENCLOUD_DBG("[Client] Error in reply id " << QString::number(_id) <<
                " (" << reply->error() << "): " << reply->errorString());
        goto err;
    }

    _response = reply->readAll();

    CLIENT_DBG("[Client] id " << QString::number(_id) << " ### <<<<< ### " << _response);

    // convert QList<QNetworkReply::RawHeaderPair> to QMap<QByteArray, QByteArray>
    foreach (QNetworkReply::RawHeaderPair header, headerPairs)
        headers[header.first] = header.second;

    emit complete(_response, headers);

err:
    disconnect(reply, NULL, NULL, NULL);
    _conns.remove(reply);
    conn->deleteLater();
    return;
}

void Client::_connectQnam()
{
    if (_qnam == NULL)
        return;

    connect(_qnam, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)), this,
            SLOT(_proxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
    connect(_qnam, SIGNAL(sslErrors(QNetworkReply *,QList<QSslError>)), this,
            SLOT(_sslErrors(QNetworkReply *,QList<QSslError>)));
    connect(_qnam, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(_finished(QNetworkReply *)));
}

Connection::Connection (Client *client, QNetworkReply *reply, int timeout, bool timeoutRetry)
    : _client(client)
    , _reply(reply)
    , _timeoutRetry(timeoutRetry)
{
    //LIBENCLOUD_TRACE;

    if (timeout > 0)
    {
        connect(&_timer, SIGNAL(timeout()), this,
                SLOT(_timeout()));

        _timer.setSingleShot(true);
        //LIBENCLOUD_TRACE_MSG("[Client] starting timer: " << timeout);
        _timer.start(timeout * 1000);
    }
}

void Connection::stop ()
{
    //LIBENCLOUD_TRACE;

    _timer.stop();
    disconnect(&_timer, NULL, NULL, NULL);
}

void Connection::_timeout ()
{
    if (_reply)
        _client->_conns.remove(_reply);

    if (_timeoutRetry)
    {
        LIBENCLOUD_DBG("[Client] retrying last request");
        _client->_sendRequest(msgType, request, data);
    }
    else
        LIBENCLOUD_EMIT(error(Error(Error::CodeClientTimeout)));

    deleteLater();
}

Connection::~Connection ()
{
    //LIBENCLOUD_TRACE;

    stop();

    if (_reply)
    {
        disconnect(_reply, NULL, NULL, NULL);
        LIBENCLOUD_DELETE (_reply);
    }
}

} // namespace libencloud
