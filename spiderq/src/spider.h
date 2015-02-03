#ifndef SPIDER_H
#define SPIDER_H

#include <stdarg.h>
#include <vector>
#include "url.h"
#include "socket.h"
#include "threads.h"
#include "confparser.h"
#include "dso.h"

/* macros */
#define MAX_MESG_LEN   1024

#define SPIDER_LEVEL_DEBUG 0
#define SPIDER_LEVEL_INFO  1
#define SPIDER_LEVEL_WARN  2
#define SPIDER_LEVEL_ERROR 3
#define SPIDER_LEVEL_CRIT  4 

static const char * LOG_STR[] = { 
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "CRIT"
};

extern Config *g_conf;

#define SPIDER_LOG(level, format, ...) do{ \
    if (level >= g_conf->log_level) {\
        time_t now = time(NULL); \
        char msg[MAX_MESG_LEN]; \
        char buf[32]; \
        sprintf(msg, format, ##__VA_ARGS__); \
        strftime(buf, sizeof(buf), "%Y%m%d %H:%M:%S", localtime(&now)); \
        fprintf(stdout, "[%s] [%s] %s\n", buf, LOG_STR[level], msg); \
        fflush(stdout); \
    } \
    if (level == SPIDER_LEVEL_ERROR) {\
        exit(-1); \
    } \
} while(0)


extern int attach_epoll_task();

#endif
