#ifndef ANET_STATS_H_
#define ANET_STATS_H_
#include <stdint.h>
namespace anet {

class StatCounter
{
public:
    StatCounter();
    ~StatCounter();
    void log();
    void clear();
    
public:    
    uint32_t _packetReadCnt;  // # packets read
    uint32_t _packetWriteCnt; // # packets written
    uint32_t _dataReadCnt;    // # bytes read
    uint32_t _dataWriteCnt;   // # bytes written

public:
    static StatCounter _gStatCounter; // ¾²Ì¬

};

#define ANET_GLOBAL_STAT StatCounter::_gStatCounter
#define ANET_COUNT_PACKET_READ(i) {ANET_GLOBAL_STAT._packetReadCnt += (i);}
#define ANET_COUNT_PACKET_WRITE(i) {ANET_GLOBAL_STAT._packetWriteCnt += (i);}
#define ANET_COUNT_DATA_READ(i) {ANET_GLOBAL_STAT._dataReadCnt += (i);}
#define ANET_COUNT_DATA_WRITE(i) {ANET_GLOBAL_STAT._dataWriteCnt += (i);}

}

#endif

