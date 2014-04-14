#ifndef _SECE_HELPERS_H_
#define _SECE_HELPERS_H_

#include <QDebug>
#include <QDateTime>
#include <QThread>

#define SECE_UNUSED(x)   (x=x)

// from <sys/syslog.h>
#define SECE_LOG_EMERG   0   /* system is unusable */
#define SECE_LOG_ALERT   1   /* action must be taken immediately */
#define SECE_LOG_CRIT    2   /* critical conditions */
#define SECE_LOG_ERR     3   /* error conditions */
#define SECE_LOG_WARNING 4   /* warning conditions */
#define SECE_LOG_NOTICE  5   /* normal but significant condition */
#define SECE_LOG_INFO    6   /* informational */
#define SECE_LOG_DEBUG   7   /* debug-level messages */

#ifndef SECE_LOGLEVEL
#define SECE_LOGLEVEL    7
#endif

#define SECE_DELETE(ptr) do { if (ptr) { delete ptr; ptr = NULL; } } while(0)

#define __SECE_NOP do {;} while (0)

#define __SECE_MSG(lev, levstr, msg) \
    do { \
        if (lev <= g_libencloudLogLev) { \
            qDebug().nospace() << qPrintable(QDateTime::currentDateTime().toString()) \
                    << " [" << QThread::currentThreadId() \
                    << "] [" << levstr << "] [" << SECE_APP << ":" << __FILE__ << ":" \
                    << __FUNCTION__ << "():" << __LINE__ << "] " << msg; \
        } \
    } while (0);

// Used to redefine __SECE_MSG, so interface must match
#define __SECE_PRINT(lev, levstr, msg) fprintf(stderr, "[%s] %s:%s:%s:%d] %s\n", SECE_PKGNAME, levstr, __FILE__, __FUNCTION__, __LINE__, qPrintable(msg))
#define SECE_PRINTF(...) fprintf(__VA_ARGS__)

#ifdef QT_NO_DEBUG_OUTPUT
  #define SECE_TRACE __SECE_NOP
  #define SECE_DBG(msg) __SECE_NOP
  #define SECE_ERR(msg) __SECE_NOP
  #define SECE_DBG_IF(cond, msg) __SECE_NOP
  #define SECE_RETURN_IF(cond, rc) do { if (cond) return rc; } while (0)
  #define SECE_RETURN_MSG_IF(cond, rc, msg) SECE_RETURN_IF(cond, rc)
  #define SECE_ERR_IF(cond) do { if (cond) goto err; } while (0)
  #define SECE_ERR_RC_IF(cond, retcode) do { if (cond) { rc = retcode; goto err; } } while (0)
  #define SECE_ERR_MSG_IF(cond, msg) SECE_ERR_IF(cond)
#else
  #if (SECE_LOGLEVEL > SECE_LOG_ERR)
    #define SECE_TRACE __SECE_MSG(SECE_LOG_DEBUG, "trace", "")
  #else
    #define SECE_TRACE __SECE_NOP
  #endif
  #if (SECE_LOGLEVEL > SECE_LOG_ERR)
    #define SECE_DBG(msg) __SECE_MSG(SECE_LOG_DEBUG, "debug", msg)
    #define SECE_DBG_IF(cond, msg) do { if (cond) SECE_DBG(msg); } while (0)
  #else
    #define SECE_DBG(msg) __SECE_NOP
    #define SECE_DBG_IF(cond, msg) __SECE_NOP
  #endif
  #define SECE_ERR(msg) __SECE_MSG(SECE_LOG_ERR, "ERROR", msg)
  #define SECE_RETURN_IF(cond, rc) do { if (cond) { SECE_ERR(""); return rc; } } while (0)
  #define SECE_RETURN_MSG_IF(cond, rc, msg) do { if (cond) { SECE_ERR(msg); return rc; } } while (0)
  #define SECE_ERR_IF(cond) do { if (cond) { SECE_ERR(""); goto err; } } while (0)
  #define SECE_ERR_RC_IF(cond, retcode) do { if (cond) { SECE_ERR(""); rc = retcode; goto err; } } while (0)
  #define SECE_ERR_MSG_IF(cond, msg) do { if (cond) { SECE_ERR(msg); goto err; } } while (0)
#endif

// New Qt-specific macros
#define SECE_EMIT_ERR(sig) do { SECE_ERR(""); emit(sig); goto err; } while (0)
#define SECE_EMIT_RETURN_IF(cond, sig, rc) do { if (cond) { SECE_ERR(""); emit(sig); return rc; } } while (0)
#define SECE_EMIT_RETURN_MSG_IF(cond, sig, msg, rc) do { if (cond) { SECE_ERR(msg); emit(sig); return rc; } } while (0)
#define SECE_EMIT_ERR_IF(cond, sig) do { if (cond) { SECE_ERR(""); emit(sig); goto err; } } while (0)
#define SECE_EMIT_ERR_MSG_IF(cond, sig, msg) do { if (cond) { SECE_ERR(msg); emit(sig); goto err; } } while (0)

#define SECE_OUT(msg) qDebug().nospace() << msg

#endif
