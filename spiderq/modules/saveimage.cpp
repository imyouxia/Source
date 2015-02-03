#include "dso.h"
#include "socket.h"
#include "url.h"
#include <fcntl.h>

/* regex pattern for parsing href */
static const char * IMG_PATTERN = "<img [^>]*src=\"\\s*\\([^ >\"]*\\)\\s*\"";

static int handler(void * data) {
    Response *r = (Response *)data;
    const size_t nmatch = 2;
    regmatch_t matchptr[nmatch];
    int len;
    regex_t re;

    if (strstr(r->header->content_type, "text/html") != NULL) {

        if (regcomp(&re, IMG_PATTERN, 0) != 0) {/* compile error */
            return MODULE_ERR;
        }

        char *p = r->body;
        while (regexec(&re, p, nmatch, matchptr, 0) != REG_NOMATCH) {
            len = (matchptr[1].rm_eo - matchptr[1].rm_so);
            p = p + matchptr[1].rm_so;
            char *tmp = (char *)calloc(len+1, 1);
            strncpy(tmp, p, len);
            tmp[len] = '\0';
            p = p + len + (matchptr[0].rm_eo - matchptr[1].rm_eo);

            char *url = attach_domain(tmp, r->url->domain);
            if (url != NULL) {
                Surl * surl = (Surl *)malloc(sizeof(Surl));
                surl->level = r->url->level;
                surl->type = TYPE_IMAGE;

                /* normalize url */
                if ((surl->url = url_normalized(url)) == NULL) {
                    free(surl);
                    continue;
                }

                if (iscrawled(surl->url)) { /* if is crawled */
                    free(surl->url);
                    free(surl);
                    continue;
                } else {
                    push_surlqueue(surl);
                }
            }
        }
    } else if (strstr(r->header->content_type, "image") != NULL) {

        char *fn = url2fn(r->url);
        int fd = -1;
        if ((fd = open(fn, O_WRONLY|O_CREAT|O_TRUNC, 0666)) < 0) {
            return MODULE_ERR;
        }
        // save image
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
    }

    return MODULE_OK;
}

static void init(Module *mod)
{
    SPIDER_ADD_MODULE_POST_HTML(mod);
}

Module saveimage = {
    STANDARD_MODULE_STUFF,
    init,
    handler
};
