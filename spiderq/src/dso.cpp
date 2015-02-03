#include <dlfcn.h>
#include "dso.h"
#include "spider.h"
#include "qstring.h"

vector<Module *> modules_pre_surl;
vector<Module *> modules_post_header;
vector<Module *> modules_post_html;

Module * dso_load(const char *path, const char *name)
{
    void *rv = NULL;
    void *handle = NULL;
    Module *module = NULL;

    char * npath = strcat2(3, path, name, ".so");

    if ((handle = dlopen(npath, RTLD_GLOBAL | RTLD_NOW)) == NULL) {	
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "Load module fail(dlopen): %s", dlerror());
    }
    if ((rv = dlsym(handle, name)) == NULL) {
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "Load module fail(dlsym): %s", dlerror());
    }
    module = (Module *)rv;
    module->init(module);

    return module;
}
