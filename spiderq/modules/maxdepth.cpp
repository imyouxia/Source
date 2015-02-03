#include "dso.h"
#include "url.h"

static int handler(void * data) {
    Surl *url = (Surl *)data;
    if (url->level > g_conf->max_depth)
        return MODULE_ERR;
    return MODULE_OK;
}

static void init(Module *mod)
{
    SPIDER_ADD_MODULE_PRE_SURL(mod);
}

Module maxdepth = {
    STANDARD_MODULE_STUFF,
    init,
    handler
};
