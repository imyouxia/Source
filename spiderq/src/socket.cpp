#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include "url.h"
#include "socket.h"
#include "threads.h"
#include "qstring.h"
#include "dso.h"

/* regex pattern for parsing href */
static const char * HREF_PATTERN = "href=\"\\s*\\([^ >\"]*\\)\\s*\"";

/* convert header string to Header object */
static Header * parse_header(char *header);

/* call modules to check header */
static int header_postcheck(Header *header);

// create socket and connect
int build_connect(int *fd, char *ip, int port)
{
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (!inet_aton(ip, &(server_addr.sin_addr))) {
        return -1;
    }

    if ((*fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    if (connect(*fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0) {
        close(*fd);
        return -1;
    }

    return 0;
}

// write http request (GET请求) to socket
int send_request(int fd, void *arg)
{
    int need, begin, n;
    char request[1024] = {0};
    Url *url = (Url *)arg;

    sprintf(request, "GET /%s HTTP/1.0\r\n"
            "Host: %s\r\n"
            "Accept: */*\r\n"
            "Connection: Keep-Alive\r\n"
            "User-Agent: Mozilla/5.0 (compatible; Qteqpidspider/1.0;)\r\n"
            "Referer: %s\r\n\r\n", url->path, url->domain, url->domain);

    need = strlen(request);
    begin = 0;
    while(need) {
        n = write(fd, request+begin, need);
        if (n <= 0) {
            if (errno == EAGAIN) { //write buffer full, delay retry
                usleep(1000);
                continue;
            }
            SPIDER_LOG(SPIDER_LEVEL_WARN, "Thread %lu send ERROR: %d", pthread_self(), n);
            free_url(url);
            close(fd);
            return -1;
        }
        begin += n;
        need -= n;
    }
    return 0;
}

// set socket nonblock
void set_nonblocking(int fd)
{
    int flag;
    if ((flag = fcntl(fd, F_GETFL)) < 0) {
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "fcntl getfl fail");
    }
    flag |= O_NONBLOCK;
    if ((flag = fcntl(fd, F_SETFL, flag)) < 0) {
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "fcntl setfl fail");
    }
}

#define HTML_MAXLEN   1024*1024

// read and deal data from socket triggered by epoll_wait
void * recv_response(void * arg)
{
    begin_thread();

    int i, n, trunc_head = 0, len = 0;
    char * body_ptr = NULL;
    evso_arg * narg = (evso_arg *)arg;
    Response *resp = (Response *)malloc(sizeof(Response));
    resp->header = NULL;
    resp->body = (char *)malloc(HTML_MAXLEN);
    resp->body_len = 0;
    resp->url = narg->url;

    regex_t re;
    if (regcomp(&re, HREF_PATTERN, 0) != 0) {/* compile error */
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "compile regex error");
    }

    SPIDER_LOG(SPIDER_LEVEL_INFO, "Crawling url: %s/%s", narg->url->domain, narg->url->path);

    while(1) {
        /* what if content-length exceeds HTML_MAXLEN? */
        n = read(narg->fd, resp->body + len, 1024);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) { 
                /**
                 * TODO: Why always recv EAGAIN?
                 * should we deal EINTR
                 */
                //SPIDER_LOG(SPIDER_LEVEL_WARN, "thread %lu meet EAGAIN or EWOULDBLOCK, sleep", pthread_self());
                usleep(100000);
                continue;
            } 
            SPIDER_LOG(SPIDER_LEVEL_WARN, "Read socket fail: %s", strerror(errno));
            break;

        } else if (n == 0) {
            /* finish reading */
            resp->body_len = len;
            if (resp->body_len > 0) {
                extract_url(&re, resp->body, narg->url);
            }
            /* deal resp->body */
            for (i = 0; i < (int)modules_post_html.size(); i++) {
                modules_post_html[i]->handle(resp);
            }

            break;

        } else {
            //SPIDER_LOG(SPIDER_LEVEL_WARN, "read socket ok! len=%d", n);
            len += n;
            resp->body[len] = '\0';

            if (!trunc_head) {
                if ((body_ptr = strstr(resp->body, "\r\n\r\n")) != NULL) {
                    *(body_ptr+2) = '\0';
                    resp->header = parse_header(resp->body);
                    if (!header_postcheck(resp->header)) {
                        goto leave; /* modulues filter fail */
                    }
                    trunc_head = 1;

                    /* cover header */
                    body_ptr += 4;
                    for (i = 0; *body_ptr; i++) {
                        resp->body[i] = *body_ptr;
                        body_ptr++;
                    }
                    resp->body[i] = '\0';
                    len = i;
                } 
                continue;
            }
        }
    }

leave:
    close(narg->fd); /* close socket */
    free_url(narg->url); /* free Url object */
    regfree(&re); /* free regex object */
    /* free resp */
    free(resp->header->content_type);
    free(resp->header);
    free(resp->body);
    free(resp);

    end_thread();
    return NULL;
}


static int header_postcheck(Header *header)
{
    unsigned int i;
    for (i = 0; i < modules_post_header.size(); i++) {
        if (modules_post_header[i]->handle(header) != MODULE_OK)
            return 0;
    }
    return 1;
}

static Header * parse_header(char *header)
{
    int c = 0;
    char *p = NULL;
    char **sps = NULL;
    char *start = header;
    Header *h = (Header *)calloc(1, sizeof(Header));

    if ((p = strstr(start, "\r\n")) != NULL) {
        *p = '\0';
        sps = strsplit(start, ' ', &c, 2);
        if (c == 3) {
            h->status_code = atoi(sps[1]);
        } else {
            h->status_code = 600; 
        }
        start = p + 2;
    }

    while ((p = strstr(start, "\r\n")) != NULL) {
        *p = '\0';
        sps = strsplit(start, ':', &c, 1);
        if (c == 2) {
            if (strcasecmp(sps[0], "content-type") == 0) {
                h->content_type = strdup(strim(sps[1]));
            }
        }
        start = p + 2;
    }
    return h;
}
