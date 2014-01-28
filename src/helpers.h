#ifndef _ENCLOUD_HELPERS_H_
#define _ENCLOUD_HELPERS_H_

#define ENCLOUD_UNUSED(x)   (x=x)

// from <sys/syslog.h>
#define ENCLOUD_LOG_EMERG   0   /* system is unusable */
#define ENCLOUD_LOG_ALERT   1   /* action must be taken immediately */
#define ENCLOUD_LOG_CRIT    2   /* critical conditions */
#define ENCLOUD_LOG_ERR     3   /* error conditions */
#define ENCLOUD_LOG_WARNING 4   /* warning conditions */
#define ENCLOUD_LOG_NOTICE  5   /* normal but significant condition */
#define ENCLOUD_LOG_INFO    6   /* informational */
#define ENCLOUD_LOG_DEBUG   7   /* debug-level messages */

#ifndef ENCLOUD_LOGLEVEL
#define ENCLOUD_LOGLEVEL    7
#endif

#define __ENCLOUD_NOP do {;} while (0)
#define __ENCLOUD_MSG(lev, levstr, msg) \
    do { \
        if (g_cfg && lev <= g_cfg->config.logLevel) { \
            qDebug().nospace() << '[' << levstr << "] [libencloud:" << __FILE__ << ":" \
                    << __FUNCTION__ << "():" << __LINE__ << "] " << msg; \
        } \
    } while (0);

// Used to redefine __ENCLOUD_MSG, so interface must match
#define __ENCLOUD_PRINT(lev, levstr, msg) fprintf(stderr, "[%s] libencloud:%s:%s:%d] %s\n", levstr, __FILE__, __FUNCTION__, __LINE__, msg)

#ifdef QT_NO_DEBUG_OUTPUT
  #define ENCLOUD_TRACE __ENCLOUD_NOP
  #define ENCLOUD_DBG(msg) __ENCLOUD_NOP
  #define ENCLOUD_ERR(msg) __ENCLOUD_NOP
  #define ENCLOUD_DBG_IF(cond, msg) __ENCLOUD_NOP
  #define ENCLOUD_RETURN_IF(cond, rc) do { if (cond) return rc; } while (0)
  #define ENCLOUD_RETURN_MSG_IF(cond, rc, msg) ENCLOUD_RETURN_IF(cond, rc)
  #define ENCLOUD_ERR_IF(cond) do { if (cond) goto err; } while (0)
  #define ENCLOUD_ERR_RC_IF(cond, retcode) do { if (cond) { rc = retcode; goto err; } } while (0)
  #define ENCLOUD_ERR_MSG_IF(cond, msg) ENCLOUD_ERR_IF(cond)
#else
  #if (ENCLOUD_LOGLEVEL > ENCLOUD_LOG_ERR)
    #define ENCLOUD_TRACE __ENCLOUD_MSG(ENCLOUD_LOG_DEBUG, "trace", "")
  #else
    #define ENCLOUD_TRACE __ENCLOUD_NOP
  #endif
  #if (ENCLOUD_LOGLEVEL > ENCLOUD_LOG_ERR)
    #define ENCLOUD_DBG(msg) __ENCLOUD_MSG(ENCLOUD_LOG_DEBUG, "debug", msg)
    #define ENCLOUD_DBG_IF(cond, msg) do { if (cond) ENCLOUD_DBG(msg); } while (0)
  #else
    #define ENCLOUD_DBG(msg) __ENCLOUD_NOP
    #define ENCLOUD_DBG_IF(cond, msg) __ENCLOUD_NOP
  #endif
  #define ENCLOUD_ERR(msg) __ENCLOUD_MSG(ENCLOUD_LOG_ERR, "ERROR", msg)
  #define ENCLOUD_RETURN_IF(cond, rc) do { if (cond) { ENCLOUD_ERR(""); return rc; } } while (0)
  #define ENCLOUD_RETURN_MSG_IF(cond, rc, msg) do { if (cond) { ENCLOUD_ERR(msg); return rc; } } while (0)
  #define ENCLOUD_ERR_IF(cond) do { if (cond) { ENCLOUD_ERR(""); goto err; } } while (0)
  #define ENCLOUD_ERR_RC_IF(cond, retcode) do { if (cond) { ENCLOUD_ERR(""); rc = retcode; goto err; } } while (0)
  #define ENCLOUD_ERR_MSG_IF(cond, msg) do { if (cond) { ENCLOUD_ERR(msg); goto err; } } while (0)
#endif

#define ENCLOUD_OUT(msg) qDebug().nospace() << msg

#endif
