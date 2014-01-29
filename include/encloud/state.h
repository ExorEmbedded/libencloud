#ifndef _ENCLOUD_STATE_H_
#define _ENCLOUD_STATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* libencloud state */
typedef enum
{
    ENCLOUD_ST_INITIALISING = 0,
    ENCLOUD_ST_CONNECTING,
    ENCLOUD_ST_CONNECTED,
    ENCLOUD_ST_INITIAL = ENCLOUD_ST_INITIALISING
}
encloud_state;

typedef void (*encloud_state_cb) (encloud_state state, void *arg);

const char *encloud_state_string (encloud_state state);

#ifdef __cplusplus
}
#endif

#endif /* _ENCLOUD_STATE_H_ */
