#include <anet/timeutil.h>
#include <sys/time.h>
#include <stdlib.h>
namespace anet {

int64_t TimeUtil::_now = 0;
const int64_t TimeUtil::MIN = (1ll << 63);
const int64_t TimeUtil::MAX = ~(1ll << 63);
const int64_t TimeUtil::PRE_MAX = ~(1ll << 63) - 1;

/*
 * 得到当前时间
 */
int64_t TimeUtil::getTime() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (static_cast<int64_t>(t.tv_sec) * static_cast<int64_t>(1000000) + static_cast<int64_t>(t.tv_usec));
}

/*
 * 设置当前时间
 */
void TimeUtil::setNow() {
    _now = getTime();
}

}
