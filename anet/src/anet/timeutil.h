#ifndef ANET_TIMEUTIL_H_
#define ANET_TIMEUTIL_H_
#include <stdint.h>
namespace anet {

class TimeUtil {
public:
    /*
     * 得到当前时间
     */
    static int64_t getTime();

    /*
     * 设置当前时间到now中
     */
    static void setNow();

public:
    static int64_t _now;
    static const int64_t MIN;
    static const int64_t MAX;
    static const int64_t PRE_MAX;
};

}

#endif
