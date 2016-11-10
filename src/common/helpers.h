#ifndef _LIBENCLOUD_PRIV_HELPERS_H_
#define _LIBENCLOUD_PRIV_HELPERS_H_

#include <QDebug>
#include <QDateTime>
#include <QThread>
#include <encloud/Logger>

#define LIBENCLOUD_UNUSED(x)   Q_UNUSED(x)

// from <sys/syslog.h>
#define LIBENCLOUD_LOG_EMERG   0   /* system is unusable */
#define LIBENCLOUD_LOG_ALERT   1   /* action must be taken immediately */
#define LIBENCLOUD_LOG_CRIT    2   /* critical conditions */
#define LIBENCLOUD_LOG_ERR     3   /* error conditions */
#define LIBENCLOUD_LOG_WARNING 4   /* warning conditions */
#define LIBENCLOUD_LOG_NOTICE  5   /* normal but significant condition */
#define LIBENCLOUD_LOG_INFO    6   /* informational */
#define LIBENCLOUD_LOG_DEBUG   7   /* debug-level messages */
#define LIBENCLOUD_LOG_MAX     LIBENCLOUD_LOG_DEBUG

#define LIBENCLOUD_FREE(ptr) do { if (ptr) { free(ptr); ptr = NULL; } } while(0)
#define LIBENCLOUD_DELETE(ptr) do { if (ptr) { delete ptr; ptr = NULL; } } while(0)
#define LIBENCLOUD_DELETE_LATER(ptr) do { if (ptr) { ptr->deleteLater(); ptr = NULL; } } while(0)

#define LIBENCLOUD_ONCE for (static bool once = true; once; once = false)

#define LIBENCLOUD_NOP do {} while (0)

// Messages are sent to remote listener if connected, otherwise passed to local Qt logger
#define __LIBENCLOUD_MSG(lev, levstr, msg) \
    do { \
        if (lev <= g_libencloudLogLev) { \
            if (libencloud::Logger::connected()) \
            { \
                static QString __s; __s.clear(); \
                static QTextStream __ts(&__s); __ts.reset(); __ts.setString(&__s);  \
                __ts << qPrintable(QDateTime::currentDateTime().toString()) \
                        << " [" << QThread::currentThreadId() \
                        << "] [" << levstr << "] [" << LIBENCLOUD_APP << ":" << __FILE__ << ":" \
                        << __FUNCTION__ << "():" << __LINE__ << "] " << msg; \
                libencloud::Logger::send(__s + "\n"); \
            } \
            else \
                qDebug().nospace() << qPrintable(QDateTime::currentDateTime().toString()) \
                        << " [" << QThread::currentThreadId() \
                        << "] [" << levstr << "] [" << LIBENCLOUD_APP << ":" << __FILE__ << ":" \
                        << __FUNCTION__ << "():" << __LINE__ << "] " << msg; \
        } \
    } while (0);

#define __LIBENCLOUD_SIMPLE_MSG(lev, levstr, msg) \
    do { \
        if (lev <= g_libencloudLogLev) { \
            if (libencloud::Logger::connected()) \
            { \
                static QString __s; __s.clear(); \
                static QTextStream __ts; __ts.reset(); __ts.setString(&__s);\
                __ts << " [" << levstr << "] " << msg; \
                libencloud::Logger::send(__s + "\n"); \
            } \
            else \
                qDebug().nospace() << " [" << levstr << "] " << msg; \
        } \
    } while (0);

#define LIBENCLOUD_GOTOERR_IF(cond) do { if (cond) { goto err; } } while (0)

// Used to redefine __LIBENCLOUD_MSG, so interface must match
#define __LIBENCLOUD_PRINT(lev, levstr, msg) fprintf(stderr, "[%s] libencloud:%s:%s:%d] %s\n", levstr, __FILE__, __FUNCTION__, __LINE__, msg)

/* lev >= LIBENCLOUD_LOG_DEBUG */
#ifndef LIBENCLOUD_DISABLE_TRACE
#  define LIBENCLOUD_TRACE __LIBENCLOUD_MSG(LIBENCLOUD_LOG_DEBUG, "trace", "")
#  define LIBENCLOUD_TRACE_MSG(msg) __LIBENCLOUD_MSG(LIBENCLOUD_LOG_DEBUG, "trace", msg)
#else
#  define LIBENCLOUD_TRACE LIBENCLOUD_NOP
#endif

/* lev >= LIBENCLOUD_LOG_INFO */
#define LIBENCLOUD_DBG(msg) __LIBENCLOUD_MSG(LIBENCLOUD_LOG_INFO, "debug", msg)
#define LIBENCLOUD_DBG_IF(cond) do { if (cond) LIBENCLOUD_DBG(""); } while (0)
#define LIBENCLOUD_DBG_MSG_IF(cond, msg) do { if (cond) LIBENCLOUD_DBG(msg); } while (0)

/* lev >= LIBENCLOUD_LOG_NOTICE */
#define LIBENCLOUD_NOTICE(msg) __LIBENCLOUD_MSG(LIBENCLOUD_LOG_NOTICE, "notice", msg)

/* lev >= LIBENCLOUD_LOG_ERR */
#define LIBENCLOUD_ERR(msg) __LIBENCLOUD_MSG(LIBENCLOUD_LOG_ERR, "ERROR", msg)
#define LIBENCLOUD_RETURN_IF(cond, rc) do { if (cond) { LIBENCLOUD_ERR(""); return rc; } } while (0)
#define LIBENCLOUD_RETURN_MSG_IF(cond, rc, msg) do { if (cond) { LIBENCLOUD_ERR(msg); return rc; } } while (0)
#define LIBENCLOUD_ERR_IF(cond) do { if (cond) { LIBENCLOUD_ERR(""); goto err; } } while (0)
#define LIBENCLOUD_ERR_RC_IF(cond, retcode) do { if (cond) { LIBENCLOUD_ERR(""); rc = retcode; goto err; } } while (0)
#define LIBENCLOUD_ERR_MSG_IF(cond, msg) do { if (cond) { LIBENCLOUD_ERR(msg); goto err; } } while (0)

/* lev >= 0 (log always) */
#define LIBENCLOUD_LOG(msg) __LIBENCLOUD_MSG(-1, "LOG", msg)

// New Qt-specific macros
#define LIBENCLOUD_EMIT(sig) do { LIBENCLOUD_DBG("emitting " << #sig); emit(sig); } while (0)
#define LIBENCLOUD_EMIT_ERR(sig) do { LIBENCLOUD_DBG(""); emit(sig); goto err; } while (0)
#define LIBENCLOUD_EMIT_RETURN_IF(cond, sig, rc) do { if (cond) { LIBENCLOUD_DBG(""); emit(sig); return rc; } } while (0)
#define LIBENCLOUD_EMIT_RETURN_MSG_IF(cond, sig, msg, rc) do { if (cond) { LIBENCLOUD_DBG(msg); emit(sig); return rc; } } while (0)
#define LIBENCLOUD_EMIT_ERR_IF(cond, sig) do { if (cond) { LIBENCLOUD_DBG(""); emit(sig); goto err; } } while (0)
#define LIBENCLOUD_EMIT_ERR_MSG_IF(cond, sig, msg) do { if (cond) { LIBENCLOUD_DBG(msg); emit(sig); goto err; } } while (0)

#define LIBENCLOUD_OUT(msg) qDebug().nospace() << msg

#endif  /* _LIBENCLOUD_PRIV_HELPERS_H_ */
