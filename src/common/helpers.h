#ifndef _LIBENCLOUD_PRIV_HELPERS_H_
#define _LIBENCLOUD_PRIV_HELPERS_H_

#include <QDebug>
#include <QDateTime>
#include <QThread>

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

#ifndef LIBENCLOUD_LOGLEVEL
#define LIBENCLOUD_LOGLEVEL    7
#endif

#define LIBENCLOUD_FREE(ptr) do { if (ptr) { free(ptr); ptr = NULL; } } while(0)
#define LIBENCLOUD_DELETE(ptr) do { if (ptr) { delete ptr; ptr = NULL; } } while(0)

#define __LIBENCLOUD_NOP do {;} while (0)
#define __LIBENCLOUD_MSG(lev, levstr, msg) \
    do { \
        if (!g_cfg || lev <= g_cfg->config.logLevel) { \
            qDebug().nospace() << qPrintable(QDateTime::currentDateTime().toString()) \
                    << " " << QThread::currentThreadId() \
                    << " [" << levstr << "] [" << LIBENCLOUD_APP << ":" << __FILE__ << ":" \
                    << __FUNCTION__ << "():" << __LINE__ << "] " << msg; \
        } \
    } while (0);

// Used to redefine __LIBENCLOUD_MSG, so interface must match
#define __LIBENCLOUD_PRINT(lev, levstr, msg) fprintf(stderr, "[%s] libencloud:%s:%s:%d] %s\n", levstr, __FILE__, __FUNCTION__, __LINE__, msg)

#ifdef QT_NO_DEBUG_OUTPUT
  #define LIBENCLOUD_TRACE __LIBENCLOUD_NOP
  #define LIBENCLOUD_DBG(msg) __LIBENCLOUD_NOP
  #define LIBENCLOUD_ERR(msg) __LIBENCLOUD_NOP
  #define LIBENCLOUD_DBG_IF(cond, msg) __LIBENCLOUD_NOP
  #define LIBENCLOUD_RETURN_IF(cond, rc) do { if (cond) return rc; } while (0)
  #define LIBENCLOUD_RETURN_MSG_IF(cond, rc, msg) LIBENCLOUD_RETURN_IF(cond, rc)
  #define LIBENCLOUD_ERR_IF(cond) do { if (cond) goto err; } while (0)
  #define LIBENCLOUD_ERR_RC_IF(cond, retcode) do { if (cond) { rc = retcode; goto err; } } while (0)
  #define LIBENCLOUD_ERR_MSG_IF(cond, msg) LIBENCLOUD_ERR_IF(cond)
#else
  #if (LIBENCLOUD_LOGLEVEL > LIBENCLOUD_LOG_ERR)
    #define LIBENCLOUD_TRACE __LIBENCLOUD_MSG(LIBENCLOUD_LOG_DEBUG, "trace", "")
  #else
    #define LIBENCLOUD_TRACE __LIBENCLOUD_NOP
  #endif
  #if (LIBENCLOUD_LOGLEVEL > LIBENCLOUD_LOG_ERR)
    #define LIBENCLOUD_DBG(msg) __LIBENCLOUD_MSG(LIBENCLOUD_LOG_DEBUG, "debug", msg)
    #define LIBENCLOUD_DBG_IF(cond, msg) do { if (cond) LIBENCLOUD_DBG(msg); } while (0)
  #else
    #define LIBENCLOUD_DBG(msg) __LIBENCLOUD_NOP
    #define LIBENCLOUD_DBG_IF(cond, msg) __LIBENCLOUD_NOP
  #endif
  #define LIBENCLOUD_ERR(msg) __LIBENCLOUD_MSG(LIBENCLOUD_LOG_ERR, "ERROR", msg)
  #define LIBENCLOUD_RETURN_IF(cond, rc) do { if (cond) { LIBENCLOUD_ERR(""); return rc; } } while (0)
  #define LIBENCLOUD_RETURN_MSG_IF(cond, rc, msg) do { if (cond) { LIBENCLOUD_ERR(msg); return rc; } } while (0)
  #define LIBENCLOUD_ERR_IF(cond) do { if (cond) { LIBENCLOUD_ERR(""); goto err; } } while (0)
  #define LIBENCLOUD_ERR_RC_IF(cond, retcode) do { if (cond) { LIBENCLOUD_ERR(""); rc = retcode; goto err; } } while (0)
  #define LIBENCLOUD_ERR_MSG_IF(cond, msg) do { if (cond) { LIBENCLOUD_ERR(msg); goto err; } } while (0)
#endif

// New Qt-specific macros
#define LIBENCLOUD_EMIT(sig) do { LIBENCLOUD_ERR("emitting " << #sig); emit(sig); } while (0)
#define LIBENCLOUD_EMIT_ERR(sig) do { LIBENCLOUD_ERR(""); emit(sig); goto err; } while (0)
#define LIBENCLOUD_EMIT_RETURN_IF(cond, sig, rc) do { if (cond) { LIBENCLOUD_ERR(""); emit(sig); return rc; } } while (0)
#define LIBENCLOUD_EMIT_RETURN_MSG_IF(cond, sig, msg, rc) do { if (cond) { LIBENCLOUD_ERR(msg); emit(sig); return rc; } } while (0)
#define LIBENCLOUD_EMIT_ERR_IF(cond, sig) do { if (cond) { LIBENCLOUD_ERR(""); emit(sig); goto err; } } while (0)
#define LIBENCLOUD_EMIT_ERROR_ERR_IF(cond, sig) do { if (cond) { LIBENCLOUD_ERR(""); emit(sig); goto err; } } while (0)
#define LIBENCLOUD_EMIT_ERR_MSG_IF(cond, sig, msg) do { if (cond) { LIBENCLOUD_ERR(msg); emit(sig); goto err; } } while (0)

#define LIBENCLOUD_OUT(msg) qDebug().nospace() << msg

#endif  /* _LIBENCLOUD_PRIV_HELPERS_H_ */
