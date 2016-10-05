#include <common/config.h>
#include <common/common.h>
#include <encloud/CookieJar>

namespace libencloud 
{

CookieJar::CookieJar (const QString &path, QObject *parent)
    : QNetworkCookieJar(parent)
    , m_restricted(true)
    , m_settings(NULL)
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_RETURN_IF (path.isEmpty(), );

    m_settings = new QSettings(path, QSettings::NativeFormat);
    LIBENCLOUD_RETURN_IF (m_settings == NULL, );

    LIBENCLOUD_DBG("[CookieJar] settings file: " << m_settings->fileName());

    load();
}

CookieJar::~CookieJar ()
{
    LIBENCLOUD_TRACE;

    LIBENCLOUD_DELETE(m_settings);
}

void CookieJar::setRestricted(bool restricted)
{
    m_restricted = restricted;
}

void CookieJar::addKnownHost(const QString &host)
{
    LIBENCLOUD_DBG("[CookieJar] host: " << host);;

    if (m_knownHosts.contains(host))
        return;

    m_knownHosts.append(host);
}

QList<QNetworkCookie> CookieJar::get()
{
    return allCookies();
}

void CookieJar::reset ()
{
    LIBENCLOUD_TRACE;

    m_settings->clear();
    m_knownHosts.clear();
    setAllCookies(QList<QNetworkCookie>());
}

QList<QNetworkCookie> CookieJar::cookiesForUrl(const QUrl &url) const
{
    //LIBENCLOUD_DBG("[CookieJar] url: " << url.toString());

    if (!m_restricted || m_knownHosts.contains(url.host()))
        // may be necessary to restrict this a little in the future..
        return QNetworkCookieJar::allCookies();

    return QNetworkCookieJar::cookiesForUrl(url);
}

bool CookieJar::setCookiesFromUrl (const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    m_settings->beginGroup("cookies");

    Q_FOREACH (QNetworkCookie cookie, cookieList)
    {   
        if (m_settings->value(cookie.name()) == cookie.value())
            continue;

        LIBENCLOUD_TRACE_MSG(qPrintable(QString("[CookieJar] set: (%1=%2)")
            .arg(QString(cookie.name()))
            .arg(QString(cookie.value()))
            ));

        m_settings->setValue(cookie.name(), cookie.value());
        m_settings->sync();
    }

    m_settings->endGroup();

    return QNetworkCookieJar::setCookiesFromUrl(cookieList, url);
}

void CookieJar::load ()
{
    LIBENCLOUD_TRACE;

    if (m_settings == NULL)
        return;

    QList<QNetworkCookie> cookies;

    m_settings->beginGroup("cookies");

    Q_FOREACH (QString key, m_settings->allKeys())
    {
        QNetworkCookie cookie;

        cookie.setName(key.toUtf8());
        cookie.setValue(m_settings->value(key).toString().toUtf8());

        LIBENCLOUD_TRACE_MSG(qPrintable(QString("[CookieJar] load: (%1=%2)")
            .arg(QString(cookie.name()))
            .arg(QString(cookie.value()))
            ));

        cookies += cookie;
    }

    m_settings->endGroup();

    setAllCookies(cookies);
}

}  // namespace libencloud
