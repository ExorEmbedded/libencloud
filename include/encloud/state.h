#ifndef _LIBENCLOUD_STATE_H_
#define _LIBENCLOUD_STATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* libencloud state */
typedef enum
{
    LIBENCLOUD_ST_INITIALISING = 0,
    LIBENCLOUD_ST_CONNECTING,
    LIBENCLOUD_ST_CONNECTED,
    LIBENCLOUD_ST_INITIAL = LIBENCLOUD_ST_INITIALISING
}
libencloud_state;

typedef void (*libencloud_state_cb) (libencloud_state state, void *arg);

const char *libencloud_state_string (libencloud_state state);

#ifdef __cplusplus
}
#endif

#endif /* _LIBENCLOUD_STATE_H_ */
