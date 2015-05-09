#ifndef ANET_LOG_H
#define ANET_LOG_H

#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include  <anet/logconfig.h>

/*
#ifndef HAVE_ALOG
#define ANET_LOG(level, ...) \
    anet::Logger::logMessage(ANET_LOG_LEVEL_##level, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#else

#include <alog/Logger.h>

#define ANET_LOG(level, format, ...) ANET_ALOG_##level(anet::Logger::anetLogger, format,##__VA_ARGS__)

#define ANET_ALOG_FATAL(logger, format, ...) ALOG_FATAL(logger, format, ##__VA_ARGS__)
#define ANET_ALOG_ERROR(logger, format, ...) ALOG_ERROR(logger, format, ##__VA_ARGS__)
#define ANET_ALOG_WARN(logger, format, ...) logger->log(alog::LOG_LEVEL_WARN,__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define ANET_ALOG_INFO(logger, format, ...) logger->log(alog::LOG_LEVEL_INFO,__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define ANET_ALOG_DEBUG(logger, format, ...) logger->log(alog::LOG_LEVEL_DEBUG,__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define ANET_ALOG_SPAM(logger, format, ...) logger->log(alog::LOG_LEVEL_TRACE1,__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

#endif
*/

#ifndef HAVE_ALOG
#define ANET_LOG(level, ...)  
#else

#include <alog/Logger.h>

#define ANET_LOG(level, format, ...)  

#define ANET_ALOG_FATAL(logger, format, ...) ALOG_FATAL(logger, format, ##__VA_ARGS__)
#define ANET_ALOG_ERROR(logger, format, ...) ALOG_ERROR(logger, format, ##__VA_ARGS__)
#define ANET_ALOG_WARN(logger, format, ...) logger->log(alog::LOG_LEVEL_WARN,__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define ANET_ALOG_INFO(logger, format, ...) logger->log(alog::LOG_LEVEL_INFO,__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define ANET_ALOG_DEBUG(logger, format, ...) logger->log(alog::LOG_LEVEL_DEBUG,__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define ANET_ALOG_SPAM(logger, format, ...) logger->log(alog::LOG_LEVEL_TRACE1,__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

#endif

namespace anet {

#define ANET_LOG_LEVEL_ERROR 0
#define ANET_LOG_LEVEL_WARN  1
#define ANET_LOG_LEVEL_INFO  2
#define ANET_LOG_LEVEL_DEBUG 3
#define ANET_LOG_LEVEL_SPAM  4

class Logger {
public:
    Logger();
    ~Logger();
    static void rotateLog(const char *filename);
    static void logMessage(int level, const char *file, int line, const char *function, const char *fmt, ...);
    static void setLogLevel(const char *level);
    static void setLogLevel(const int level);
    static void logSetup();
    static void logTearDown();
#ifdef HAVE_ALOG
    static alog::Logger *anetLogger;
#endif
};

}
#endif/*ANET_LOG_H*/
