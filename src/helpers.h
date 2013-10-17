#ifndef _ECE_HELPERS_H_
#define _ECE_HELPERS_H_

#define ECE_UNUSED(x)   (x=x)

// from <sys/syslog.h>
#define ECE_LOG_EMERG   0   /* system is unusable */
#define ECE_LOG_ALERT   1   /* action must be taken immediately */
#define ECE_LOG_CRIT    2   /* critical conditions */
#define ECE_LOG_ERR     3   /* error conditions */
#define ECE_LOG_WARNING 4   /* warning conditions */
#define ECE_LOG_NOTICE  5   /* normal but significant condition */
#define ECE_LOG_INFO    6   /* informational */
#define ECE_LOG_DEBUG   7   /* debug-level messages */

#ifndef ECE_LOGLEVEL
#define ECE_LOGLEVEL    7
#endif

#define __ECE_NOP do {;} while (0)
#define __ECE_MSG(lev, levstr, msg) \
    do { \
        if (g_cfg && lev <= g_cfg->config.logLevel) { \
            qDebug().nospace() << '[' << levstr << "] [libece:" << __FILE__ << ":" \
                    << __FUNCTION__ << "():" << __LINE__ << "] " << msg; \
        } \
    } while (0);

// Used to redefine __ECE_MSG, so interface must match
#define __ECE_PRINT(lev, levstr, msg) fprintf(stderr, "[%s] libece:%s:%s:%d] %s\n", levstr, __FILE__, __FUNCTION__, __LINE__, msg)

#ifdef QT_NO_DEBUG_OUTPUT
  #define ECE_TRACE __ECE_NOP
  #define ECE_DBG(msg) __ECE_NOP
  #define ECE_ERR(msg) __ECE_NOP
  #define ECE_DBG_IF(cond, msg) __ECE_NOP
  #define ECE_RETURN_IF(cond, rc) do { if (cond) return rc; } while (0)
  #define ECE_RETURN_MSG_IF(cond, rc, msg) ECE_RETURN_IF(cond, rc)
  #define ECE_ERR_IF(cond) do { if (cond) goto err; } while (0)
  #define ECE_ERR_RC_IF(cond, retcode) do { if (cond) { rc = retcode; goto err; } } while (0)
  #define ECE_ERR_MSG_IF(cond, msg) ECE_ERR_IF(cond)
#else
  #if (ECE_LOGLEVEL > ECE_LOG_ERR)
    #define ECE_TRACE __ECE_MSG(ECE_LOG_DEBUG, "trace", "")
  #else
    #define ECE_TRACE __ECE_NOP
  #endif
  #if (ECE_LOGLEVEL > ECE_LOG_ERR)
    #define ECE_DBG(msg) __ECE_MSG(ECE_LOG_DEBUG, "debug", msg)
    #define ECE_DBG_IF(cond, msg) do { if (cond) ECE_DBG(msg); } while (0)
  #else
    #define ECE_DBG(msg) __ECE_NOP
    #define ECE_DBG_IF(cond, msg) __ECE_NOP
  #endif
  #define ECE_ERR(msg) __ECE_MSG(ECE_LOG_ERR, "ERROR", msg)
  #define ECE_RETURN_IF(cond, rc) do { if (cond) { ECE_ERR(""); return rc; } } while (0)
  #define ECE_RETURN_MSG_IF(cond, rc, msg) do { if (cond) { ECE_ERR(msg); return rc; } } while (0)
  #define ECE_ERR_IF(cond) do { if (cond) { ECE_ERR(""); goto err; } } while (0)
  #define ECE_ERR_RC_IF(cond, retcode) do { if (cond) { ECE_ERR(""); rc = retcode; goto err; } } while (0)
  #define ECE_ERR_MSG_IF(cond, msg) do { if (cond) { ECE_ERR(msg); goto err; } } while (0)
#endif

#define ECE_OUT(msg) qDebug().nospace() << msg

#endif
