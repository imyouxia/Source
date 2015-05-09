#include <anet/log.h>
#include <config.h>
#ifdef HAVE_ALOG
#include <alog/Logger.h>
#include <alog/Configurator.h>
#define LOGLEVEL_DIFF 2
#endif

namespace anet {
static const char *g_errstr[] = {"ERROR","WARN","INFO","DEBUG","SPAM"
                          };

#ifndef HAVE_ALOG
static int g_level = 1;
#else
alog::Logger *Logger::anetLogger = alog::Logger::getLogger("anet");
#endif

Logger::Logger() {}

Logger::~Logger() {}

void Logger::logSetup() {
#ifdef HAVE_ALOG
    alog::Configurator::configureRootLogger();
#endif
}

void Logger::logTearDown() {
#ifdef HAVE_ALOG
    alog::Logger::shutdown();
#endif
}

void Logger::setLogLevel(const char *level) {
    if (level == NULL) return;
    int l = sizeof(g_errstr)/sizeof(char*);
    for (int i=0; i<l; i++) {
        if (strcasecmp(level, g_errstr[i]) == 0) {
#ifdef HAVE_ALOG
            uint32_t tmp = i + LOGLEVEL_DIFF;
            if (tmp >= alog::LOG_LEVEL_COUNT) {
                tmp = alog::LOG_LEVEL_DEBUG;
            }
            anetLogger->setLevel(tmp);
#else
            g_level = i;
#endif
            break;
        }
    }
}

void Logger::setLogLevel(const int level) {
#ifdef HAVE_ALOG
    anetLogger->setLevel(level + LOGLEVEL_DIFF);
#else
    if (level < 0) {
        g_level = 0;
    } else {
        g_level = level;
    }
#endif
}

void Logger::logMessage(int level, const char *file, int line, const char *function, const char *fmt, ...) {
#ifndef HAVE_ALOG
    if (level>g_level) return;

    char buffer[1024];
    time_t t;
    time(&t);
    struct tm *tm = ::localtime((const time_t*)&t);

    int size = snprintf(buffer,1024,"[%04d-%02d-%02d %02d:%02d:%02d] %-5s %s (%s:%d) %s\n",
                        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
                        tm->tm_hour, tm->tm_min, tm->tm_sec,
                        g_errstr[level], function, file, line, fmt);
    // 去掉过多的换行
    while (buffer[size-2] == '\n') size --;
    buffer[size] = '\0';

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, buffer, args);
    fflush(stderr);
    va_end(args);
#endif
}


void Logger::rotateLog(const char *filename) {
    if (access(filename, R_OK) == 0) {
        char oldLogFile[256];
        time_t t;
        time(&t);
        struct tm *tm = ::localtime((const time_t*)&t);
        sprintf(oldLogFile, "%s.%04d%02d%02d%02d%02d%02d",
                filename, tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
        rename(filename, oldLogFile);
    }
    int fd = open(filename, O_RDWR | O_CREAT | O_APPEND, 0640);
    dup2(fd, 1);
    dup2(fd, 2);
    close(fd);
}
}

/////////////
