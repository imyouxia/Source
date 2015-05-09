#ifndef ANET_PACKET_H_
#define ANET_PACKET_H_
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

namespace anet {

#define ANET_PACKET_FLAG 0x416e4574  // AnEt
#define CONNECTION_CLOSE "close"
#define CONNECTION_KEEP_ALIVE "Keep-Alive"
class DataBuffer;
class Channel;
class PacketHeader {
public:
    uint32_t _chid;         // 通道ID
    int32_t _pcode;             // 数据包类型
    int32_t _dataLen;           // 数据包body长度(除头信息外)
};

class Packet {
    friend class PacketQueue;

public:
    /*
     * 构造函数, 传包类型
     */
    Packet();

    /*
     * 析构函数
     */
    virtual ~Packet();

    /*
     * 设置ChannelID
     */
    void setChannelId(uint32_t chid) {
        _packetHeader._chid = chid;
    }

    /*
     * 得到Channel ID
     */
    uint32_t getChannelId() {
        return _packetHeader._chid;
    }

    /*
     * 得到数据包header info
     */
    PacketHeader *getPacketHeader() {
        return &_packetHeader;
    }

    /*
     * 设置数据包header info
     */
    void setPacketHeader(PacketHeader *header) {
        if (header) {
            memcpy(&_packetHeader, header, sizeof(PacketHeader));
        }
    }

  /**
   * The Packet is freed through this->free(). In most situation,
   * packet are freed in TCPConnection::writeData() atomatically
   * through this function.
   */
  virtual void free() {
    delete this;
  }

    /*
     * 是否数据包
     */
    virtual bool isRegularPacket() {
        return true;
    }

    /** 
     * Write data into DataBuffer. This function is called by 
     * Streamer. Streamer uses this function write data into 
     * DataBuffer. The packet will be deleted through packet->free()
     * after this function return.
     *
     * @param output target DataBuffer
     * @return Return true when decode success! Else return false.
     */
    virtual bool encode(DataBuffer *output) = 0;

    /**
     * Read data form DataBuffer according to the information in
     * PacketHeader and construct packet. The DataBuffer contains
     * all the data you need. PacketHeader records dataLength and
     * some other information.
     *
     * @param input source data
     * @param header packet information
     * @return Return true when decode success! Else return false.
     */
    virtual bool decode(DataBuffer *input, PacketHeader *header) = 0;

    /*
     * 超时时间
     */
    int64_t getExpireTime() {
        return _expireTime;   
    }
    
    /*
     * 设置过期时间
     * 
     * @param milliseconds 毫秒数, 0为永不过期
     */
    void setExpireTime(int milliseconds);
    
    /*
     * 设置Channel
     */
    void setChannel(Channel *channel);

    /*
     * 得到Channel
     */
    Channel *getChannel()
    {
        return _channel;
    }    
    
    /*
     * 得到next
     */
    Packet *getNext()
    {
        return _next;
    }

protected:
    PacketHeader _packetHeader; // 数据包的头信息
    int64_t _expireTime;        // 到期时间
    Channel *_channel;
    
    Packet *_next;              // 用在packetqueue链表
};

}

#endif /*PACKET_H_*/
