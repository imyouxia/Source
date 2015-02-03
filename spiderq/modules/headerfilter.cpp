#include "dso.h"
#include "socket.h"

static int handler(void * data) {
    Header *h = (Header *)data;
    int i = 0;

    /* skip if not 2xx */
    if (h->status_code < 200 || h->status_code >= 300)
        return MODULE_ERR;

    if (h->content_type != NULL) {
        if (strstr(h->content_type, "text/html") != NULL)
            return MODULE_OK;

        for (i = 0; i < g_conf->accept_types.size(); i++) {
            if (strstr(h->content_type, g_conf->accept_types[i]) != NULL)
                return MODULE_OK;
        }

        return MODULE_ERR;
    }

    return MODULE_OK;
}

static void init(Module *mod)
{
    SPIDER_ADD_MODULE_POST_HEADER(mod);
}

Module headerfilter = {
    STANDARD_MODULE_STUFF,
    init,
    handler
};
