#ifndef _ECE_TEST_H_
#define _ECE_TEST_H_

#include <stdio.h>

#include "helpers.h"
#include "config.h"

// if compiling with gcc, don't depend on Qt for debug
#ifndef __cplusplus
  #undef __ECE_MSG
  #define __ECE_MSG(lev,msg) __ECE_PRINT(lev, msg)
#else
  #include <QDebug>
#endif

#define TEST_ZERO(z) ECE_ERR_IF(z)
#define TEST_EQUALS(x,y) ECE_ERR_IF(x!=y)
#define TEST_MSG(msg) fprintf(stderr, "[test:%s:%s:%d] %s\n", \
        __FILE__, __FUNCTION__, __LINE__, msg)
#define TEST_TRACE TEST_MSG("")

#endif
