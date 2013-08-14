#ifndef _ECE_TEST_H_
#define _ECE_TEST_H_

#define TEST_MSG(msg) fprintf(stderr, "[test:%s:%s:%d] %s\n", \
        __FILE__, __FUNCTION__, __LINE__, msg)
#define TEST_TRACE TEST_MSG("")

#endif
