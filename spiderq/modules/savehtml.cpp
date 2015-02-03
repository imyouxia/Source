#include "dso.h"
#include "socket.h"
#include <fcntl.h>

static int handler(void * data) {
    Response *r = (Response *)data;
    
    if (strstr(r->header->content_type, "text/html") == NULL)
        return MODULE_ERR;

    char *fn = url2fn(r->url);
    int fd = -1;
    if ((fd = open(fn, O_WRONLY|O_CREAT|O_TRUNC, 0666)) < 0) {
        return MODULE_ERR;
    }

    int left = r->body_len;
    int n = -1;
    while (left) {
        if ((n = write(fd, r->body, left)) < 0) {
            // error
            close(fd);
            unlink(fn);
            free(fn);
            return MODULE_ERR;
        } else {
            left -= n;
        }
    }
    close(fd);
    free(fn);
    return MODULE_OK;
}

static void init(Module *mod)
{
    SPIDER_ADD_MODULE_POST_HTML(mod);
}

Module savehtml = {
    STANDARD_MODULE_STUFF,
    init,
    handler
};
