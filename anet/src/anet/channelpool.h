#ifndef ANET_CHANNEL_POOL_H_
#define ANET_CHANNEL_POOL_H_
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <anet/atomic.h>
#include <anet/threadmutex.h>
#include <ext/hash_map>
#include <list>
namespace anet {
const size_t CHANNEL_CLUSTER_SIZE = 64lu;
 class Channel;

class ChannelPool {
public:	
    ChannelPool();
    ~ChannelPool();

    static const uint32_t AUTO_CHANNEL_ID_MAX;
    static const uint32_t HTTP_CHANNLE_ID;
    static const uint32_t ADMIN_CHANNLE_ID;
    /**
     * Allocate a new channel.
     * 
     * @param chid The desired channel id for the channel just allocated. If
     * chid is less than or equal to AUTO_CHANNEL_ID_MAX, the channel id of
     * the new channel will be generated automatically, which is larger than
     * 0 and less than AUTO_CHANNEL_ID_MAX. if chid is greater than
     * AUTO_CHANNEL_ID_MAX,  the channel id of the new channel will be chid.
     */
    Channel *allocChannel(uint32_t chid = 0);


    bool freeChannel(Channel *channel);

    /*
     * 查找一下channel
     *
     * @param id: channel id
     * @return Channel
     */
    Channel* findChannel(uint32_t id);

    /*
     * 从useList中找出超时的channel的list,并把hashmap中对应的删除
     *
     * @param now: 当前时间
     */
    Channel* getTimeoutList(int64_t now);

    /*_mutex
     * 把addList的链表加入到freeList中
     *
     * @param addList被加的list
     */
    bool appendFreeList(Channel *addList);
    
    /*
     * 被用链表的长度
     */
    size_t getUseListCount() {
        return _useListCount;
    }

private:
    __gnu_cxx::hash_map<uint32_t, Channel*> _useMap; // 使用的map
    std::list<Channel*> _clusterList;                // cluster list
    ThreadMutex _mutex;

    Channel *_freeListHead;             // 空的链表
    Channel *_freeListTail;
    Channel *_useListHead;              // 被使用的链表
    Channel *_useListTail;
    size_t _useListCount;                  // 被用链表的长度

    static atomic_t _globalChannelId;   // 生成统一的id

    friend class CHANNELPOOLTF;    //add by hua.huangrh
};

}

#endif /*CHANNEL_POOL_H_*/
