#ifndef _NET_H_
#define _NET_H_

#include <pal.h>

enum
{
    NETM_CONNECTED,
    NETM_GETHEADERS,
    NETM_GETPAYLOAD,
    NETM_RESPONSE,
    NETM_RECEIVED,
    NETM_COMPLETE,
    NETM_ERROR,
};

typedef struct
{
    unsigned status;
    char     status_text[PATH_MAX];
    unsigned content_length;
} net_response_param;

typedef struct
{
    size_t         size;
    const uint8_t *data;
} net_received_param;

typedef int(net_proc)(int msg, void *param, void *data);

extern bool
net_start(void);

extern void
net_stop(void);

extern bool
net_connect(const char *url, net_proc *proc, void *data);

extern bool
net_request(const char *method, const char *url);

void
net_close(void);

#endif
