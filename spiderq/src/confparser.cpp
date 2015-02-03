#include "spider.h"
#include "qstring.h"
#include "confparser.h"

#define INF 0x7FFFFFFF

Config * initconfig()
{
    Config *conf = (Config *)malloc(sizeof(Config));

    conf->max_job_num = 10;
    conf->seeds = NULL;
    conf->include_prefixes = NULL;
    conf->exclude_prefixes = NULL;
    conf->logfile = NULL;
    conf->log_level = 0;
    conf->max_depth = INF;
    conf->make_hostdir = 0;
    conf->module_path = NULL;
    conf->stat_interval = 0;
    //conf->modules

    return conf;
}

void loadconfig(Config *conf)
{
    FILE *fp = NULL;
    char buf[MAX_CONF_LEN+1];
    int argc = 0;
    char **argv = NULL;
    int linenum = 0;
    char *line = NULL;
    const char *err = NULL;

    if ((fp = fopen(CONF_FILE, "r")) == NULL) {
        SPIDER_LOG(SPIDER_LEVEL_ERROR, "Can't load conf_file %s", CONF_FILE);	
    } 

    while (fgets(buf, MAX_CONF_LEN+1, fp) != NULL) {
        linenum++;
        line = strim(buf);

        if (line[0] == '#' || line[0] == '\0') continue;

        argv = strsplit(line, '=', &argc, 1);
        if (argc == 2) {
            if (strcasecmp(argv[0], "max_job_num") == 0) {
                conf->max_job_num = atoi(argv[1]);
            } else if (strcasecmp(argv[0], "logfile") == 0) {
                conf->logfile = strdup(argv[1]);
            } else if (strcasecmp(argv[0], "include_prefixes") == 0) {
                conf->include_prefixes = strdup(argv[1]);
            } else if (strcasecmp(argv[0], "exclude_prefixes") == 0) {
                conf->exclude_prefixes = strdup(argv[1]);
            } else if (strcasecmp(argv[0], "seeds") == 0) {
                conf->seeds = strdup(argv[1]);
            } else if (strcasecmp(argv[0], "module_path") == 0) {
                conf->module_path = strdup(argv[1]);
            } else if (strcasecmp(argv[0], "load_module") == 0) {
                conf->modules.push_back(strdup(argv[1]));
            } else if (strcasecmp(argv[0], "log_level") == 0) {
                conf->log_level = atoi(argv[1]);
            } else if (strcasecmp(argv[0], "max_depth") == 0) {
                conf->max_depth = atoi(argv[1]);
            } else if (strcasecmp(argv[0], "stat_interval") == 0) {
                conf->stat_interval = atoi(argv[1]);
            } else if (strcasecmp(argv[0], "make_hostdir") == 0) {
                conf->make_hostdir = yesnotoi(argv[1]);
            } else if (strcasecmp(argv[0], "accept_types") == 0) {
                conf->accept_types.push_back(strdup(argv[1]));
            } else {
                err = "Unknown directive"; goto conferr;
            }
        } else {
            err = "directive must be 'key=value'"; goto conferr;
        }

    }
    return;

conferr:
    SPIDER_LOG(SPIDER_LEVEL_ERROR, "Bad directive in %s[line:%d] %s", CONF_FILE, linenum, err);	
}
