#ifndef _LIBENCLOUD_TEST_H_
#define _LIBENCLOUD_TEST_H_

#include <QtTest>
#include <stdio.h>
#include <common/helpers.h>
#include <common/config.h>

// if compiling with gcc, don't depend on Qt for debug
#ifndef __cplusplus
  #undef __LIBENCLOUD_MSG
  #define __LIBENCLOUD_MSG(lev,msg) __LIBENCLOUD_PRINT(lev, msg)
#else
  #include <QDebug>
#endif

#define TEST_ZERO(z) LIBENCLOUD_ERR_IF(z)

// test retrying in tout minutes
#define TEST_ZERO_RETRY(z, tout) \
    do { \
        int _i; \
        for (_i = 0; _i < tout; _i++) \
        { \
            z; \
            if (rc == 0) \
                break; \
            fprintf(stderr, "# ERROR - retrying in a minute (i=%d, tout=%d)\n", _i, tout); \
            sleep(60); \
        } \
        if (_i == tout) \
            TEST_ZERO(rc); \
    } while (0);

#define TEST_EQUALS(x,y) LIBENCLOUD_ERR_IF(x!=y)
#define TEST_MSG(msg) fprintf(stderr, "[test:%s:%s:%d] %s\n", \
        __FILE__, __FUNCTION__, __LINE__, msg)
#define TEST_TRACE TEST_MSG("")

#endif
