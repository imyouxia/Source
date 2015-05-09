#include <anet/log.h>
#include <anet/stats.h>

namespace anet {
    
StatCounter StatCounter::_gStatCounter;

/*
 * 构造函数
 */
StatCounter::StatCounter()
{
    clear();
}

/*
 * 析构函数
 */
StatCounter::~StatCounter()
{
}

/*
 * 把stat写到log中
 */
void StatCounter::log()
{
    ANET_LOG(INFO, "_packetReadCnt: %u, _packetWriteCnt: %u, _dataReadCnt: %u, _dataWriteCnt: %u",
        _packetReadCnt, _packetWriteCnt, _dataReadCnt, _dataWriteCnt);
}

/*
 * 清空
 */
void StatCounter::clear()
{
    _packetReadCnt = 0;
    _packetWriteCnt = 0;
    _dataReadCnt = 0;
    _dataWriteCnt = 0;
}

}
