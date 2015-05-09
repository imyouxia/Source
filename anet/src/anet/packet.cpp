#include <anet/packet.h>
#include <anet/timeutil.h>
#include <iostream>
#include <anet/databuffer.h>
#include <anet/channel.h>


namespace anet {

/*
 * 构造函数, 传包类型
 */
Packet::Packet() {
    _next = NULL;
    _channel = NULL;
    _expireTime = 0;
    memset(&_packetHeader, 0, sizeof(PacketHeader));
}

/*
 * 析构函数
 */
Packet::~Packet() {
}

/*
 * 设置Channel
 */
void Packet::setChannel(Channel *channel) {
    if (channel) {
        _channel = channel;
        _packetHeader._chid = channel->getId();
    }
}
    
/*
 * 设置过期时间
 *
 * @param milliseconds 毫秒数, 0为永不过期
 */
void Packet::setExpireTime(int milliseconds) {
    if (milliseconds == 0) {
        _expireTime = TimeUtil::PRE_MAX;;
    } else {/**@todo this design is not testing friendly*/
        _expireTime = TimeUtil::getTime() + static_cast<int64_t>(milliseconds) * static_cast<int64_t>(1000);
    }
}

}
