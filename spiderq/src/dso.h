#ifndef DSO_H
#define DSO_H

#include <vector>
using namespace std;

#define MODULE_OK  0
#define MODULE_ERR 1

#define MAGIC_MAJOR_NUMBER 20130101
#define MAGIC_MINOR_NUMBER 0


#define STANDARD_MODULE_STUFF MAGIC_MAJOR_NUMBER, \
                              MAGIC_MINOR_NUMBER, \
                              __FILE__

typedef struct Module{
    int          version;
    int          minor_version;
    const char  *name;
    void (*init)(Module *);
    int (*handle)(void *);
} Module;

/* The modules in this queue are used before pushing Url object into surl_queue */
extern vector<Module *> modules_pre_surl;

#define SPIDER_ADD_MODULE_PRE_SURL(module) do {\
    modules_pre_surl.push_back(module); \
} while(0)


/* The modules in this queue are used after parsing out http header */
extern vector<Module *> modules_post_header;

#define SPIDER_ADD_MODULE_POST_HEADER(module) do {\
    modules_post_header.push_back(module); \
} while(0)


/* The modules in this queue are used after finishing read html */
extern vector<Module *> modules_post_html;

#define SPIDER_ADD_MODULE_POST_HTML(module) do {\
    modules_post_html.push_back(module); \
} while(0)

/* Dynamic load modules while spiderq is starting */
extern Module * dso_load(const char *path, const char *name);

#endif
