#ifndef _ECE_TEST_H_
#define _ECE_TEST_H_

#include "helpers.h"
// don't depend on Qt for debug
#undef __ECE_MSG
#define __ECE_MSG(lev,msg) __ECE_PRINT(lev, msg)

#define TEST_ZERO(z) ECE_ERR_IF(z)
#define TEST_EQUALS(x,y) ECE_ERR_IF(x!=y)
#define TEST_MSG(msg) fprintf(stderr, "[test:%s:%s:%d] %s\n", \
        __FILE__, __FUNCTION__, __LINE__, msg)
#define TEST_TRACE TEST_MSG("")

#endif
