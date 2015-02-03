#include "dso.h"
#include "url.h"
#include "qstring.h"
#include <vector>
using namespace std;

typedef struct Dlnode {
    char *prefix;
    int   len;
} Dlnode;

static vector<Dlnode *> include_nodes;
static vector<Dlnode *> exclude_nodes;

static int handler(void * data) {
    unsigned int i;
    Surl *url = (Surl *)data;

    /* rules does NOT work for seeds */
    if (url->level == 0 || url->type != TYPE_HTML)
        return MODULE_OK;

    /* if include_nodes is NOT empty and the url match none, return MODULE_ERR */ 
    for (i = 0; i < include_nodes.size(); i++) {
        if (strncmp(url->url, include_nodes[i]->prefix, include_nodes[i]->len) == 0)
            break;
    }
    if (i >= include_nodes.size() && include_nodes.size() > 0)
        return MODULE_ERR;

    /* if exclude_nodes is NOT empty and the url match one, return MODULE_ERR */ 
    for (i = 0; i < exclude_nodes.size(); i++) {
        if (strncmp(url->url, exclude_nodes[i]->prefix, exclude_nodes[i]->len) == 0)
            return MODULE_ERR;
    }

    return MODULE_OK;
}

static void init(Module *mod)
{
    SPIDER_ADD_MODULE_PRE_SURL(mod);

    if (g_conf->include_prefixes != NULL) {
        int c = 0;
        char ** ss = strsplit(g_conf->include_prefixes, ',', &c, 0);
        while (c--) {
            Dlnode * n = (Dlnode *)malloc(sizeof(Dlnode));
            n->prefix = strim(ss[c]);
            n->len = strlen(n->prefix);
            include_nodes.push_back(n);
        }
    }
    if (g_conf->exclude_prefixes != NULL) {
        int c = 0;
        char ** ss = strsplit(g_conf->exclude_prefixes, ',', &c, 0);
        while (c--) {
            Dlnode * n = (Dlnode *)malloc(sizeof(Dlnode));
            n->prefix = strim(ss[c]);
            n->len = strlen(n->prefix);
            exclude_nodes.push_back(n);
        }
    } 
}

Module domainlimit = {
    STANDARD_MODULE_STUFF,
    init,
    handler
};
