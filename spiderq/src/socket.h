#ifndef QSOCKET_H
#define QSOCKET_H

#include "url.h"

/* struct for http header */
typedef struct Header {
    char      *content_type;
    int        status_code;
} Header;

typedef struct Response {
    Header *header;
    char   *body;
    int     body_len;

    struct Url    *url;
} Response;

/* create socket and connect */
extern int build_connect(int *fd, char *ip, int port);

/* write http request to socket */
extern int send_request(int fd, void *arg);

/* set socket nonblock */
extern void set_nonblocking(int fd);

/* read and deal data from socket triggered by epoll_wait */
extern void * recv_response(void *arg);

#endif
