#ifndef _LIBENCLOUD_PRIV_COMMON_H_
#define _LIBENCLOUD_PRIV_COMMON_H_

#include <common/defaults.h>
#include <common/helpers.h>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif

namespace libencloud {

/* Protocol modes */
enum ProtocolType
{
    ProtocolTypeInit,
    ProtocolTypeOp
};

/* Message types */
enum MessageType
{
    MessageTypeRetrInfo,
    MessageTypeRetrCert,
    MessageTypeRetrConf
};

} // namespace libencloud

QString getQueryItemValue(const QUrl &url, const QString &param);
void removeQueryItem(QUrl &url, const QString &param);
void addQueryItem(QUrl &url, const QString &param, const QString &value);
void setEncodedQuery(QUrl &url, const QString &query);
QList<QPair<QString, QString>> queryItems(const QUrl &url);

#endif  /* _LIBENCLOUD_PRIV_COMMON_H_ */
