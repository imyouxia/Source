#ifndef CONFPARSER_H
#define CONFPARSER_H

#include <vector>
using namespace std;

#define MAX_CONF_LEN  1024
#define CONF_FILE     "spiderq.conf"

/* see the spiderq.conf to get meaning for each member variable below */
typedef struct Config {
    int              max_job_num;
    char            *seeds;
    char            *include_prefixes; 
    char            *exclude_prefixes; 
    char            *logfile; 
    int              log_level;
    int              max_depth;
    int              make_hostdir;
    int              stat_interval;

    char *           module_path;
    vector<char *>   modules;
    vector<char *>   accept_types;
};

/* give default values to member variables in struct Config */
extern Config * initconfig();

/* load configuration in conf file to Config object */
extern void loadconfig(Config *conf);


#endif
