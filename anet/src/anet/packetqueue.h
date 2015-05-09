#ifndef ANET_PACKET_QUEUE_H_
#define ANET_PACKET_QUEUE_H_
#include <sys/types.h>
namespace anet {
  class Packet;
  class PacketQueue {
public:
    /*
     * 构造函数
     */
    PacketQueue();
    
    /*
     * 析构函数
     */
    ~PacketQueue();
    
    /*
     * 出链
     */
    Packet *pop();
    
    /*
     * 入链
     */
    void push(Packet *packet);
  
    /*
     * 长度
     */ 
    size_t size();

    /*
     * 是否为空
     */
    bool empty();

    /**
     * move all entries of current queue to the tail of destQueue 
     */
    void moveTo(PacketQueue *destQueue);

    /**
     * move all entries of  srcQueue back to the head of current queue
     */
    void moveBack(PacketQueue *srcQueue);

    /*
     * 得到超时的packet
     */
    Packet *getTimeoutList(int64_t now);

protected:
    Packet *_head;  // 链头
    Packet *_tail;  // 链尾
    size_t _size;      // 元素数量
};

}

#endif

