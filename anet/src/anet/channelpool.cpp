#include <ext/hash_map>
#include <list>
#include <anet/threadmutex.h>
#include <anet/channelpool.h>
#include <anet/channel.h>
namespace anet {
const uint32_t ChannelPool::AUTO_CHANNEL_ID_MAX = 0x0FFFFFFFu;
const uint32_t ChannelPool::HTTP_CHANNLE_ID     = 0x10000000u;
const uint32_t ChannelPool::ADMIN_CHANNLE_ID    = 0x10000001u;

atomic_t ChannelPool::_globalChannelId = {0};

/*
 * 构造函数
 */
ChannelPool::ChannelPool() {
    _freeListHead = NULL;
    _freeListTail = NULL;
    _useListHead = NULL;
    _useListTail = NULL;
    _useListCount = 0;
}

/*
 * 析构函数
 */
ChannelPool::~ChannelPool() {
     std::list<Channel*>::iterator it = _clusterList.begin();

     for (;it != _clusterList.end(); it++) {
         delete[] *it;
     }
}

/*
 * 得到一个新的channel
 *
 * @return 一个Channel
 */
Channel *ChannelPool::allocChannel(uint32_t chid) {
    Channel *channel = NULL;
    _mutex.lock();
    if (_freeListHead == NULL) { // 如果是空，新分配一些放到freeList中
        assert(CHANNEL_CLUSTER_SIZE>2);
        Channel *channelCluster = new Channel[CHANNEL_CLUSTER_SIZE];
		
		//add by hua.huangrh 08.7.21
		assert(channelCluster != NULL);
		_clusterList.push_back(channelCluster);
		//end add by hua.huangrh
		
        _freeListHead = _freeListTail = &channelCluster[1];
        for (size_t i = 2; i < CHANNEL_CLUSTER_SIZE; i++) {
            _freeListTail->_next = &channelCluster[i];
            channelCluster[i]._prev = _freeListTail;
            _freeListTail = _freeListTail->_next;
        }
        _freeListHead->_prev = NULL;
        _freeListTail->_next = NULL;
        channel = &channelCluster[0];   // 把第一个元素拿过来直接用, 不放到freelist中
    } else {
        // 从链头取出
        channel = _freeListHead;
        _freeListHead = _freeListHead->_next;
        if (_freeListHead != NULL) {
            _freeListHead->_prev = NULL;
        } else {//add by hua.huangrh 08.7.22 11:09
            _freeListTail = NULL;
        }
    }

    // 把channel放到_useList中
    channel->_prev = _useListTail;
    channel->_next = NULL;
    if (_useListTail == NULL) {
        _useListHead = channel;
    } else {
        _useListTail->_next = channel;
    }
    _useListTail = channel;
    _useListCount ++;

    // 生成id
    uint32_t id = chid;
    if (id <= AUTO_CHANNEL_ID_MAX) {
        do {
        id = atomic_add_return(1, &_globalChannelId);
        id &= AUTO_CHANNEL_ID_MAX;
        } while (0 == id);
    }

    channel->_id = id;
    channel->_expireTime = 0;
    channel->_handler = NULL;
    channel->_args = NULL;

    // 把channel放到hashmap中
    _useMap[id] = channel;
    _mutex.unlock();

    return channel;
}

/*
 * 释放一个channel
 *
 * @param channel: 要释放的channel
 * @return
 */
bool ChannelPool::freeChannel(Channel *channel) {
	if (channel == NULL) return false;
    _mutex.lock();
    _useMap.erase(channel->_id);

    // 从_userList删除
    if (channel == _useListHead) { // head
        _useListHead = channel->_next;
    }
    if (channel == _useListTail) { // tail
        _useListTail = channel->_prev;
    }
    if (channel->_prev != NULL)
        channel->_prev->_next = channel->_next;
    if (channel->_next != NULL)
        channel->_next->_prev = channel->_prev;
    _useListCount --;

    // 加入到_freeList
    channel->_prev = _freeListTail;
    channel->_next = NULL;
    if (_freeListTail == NULL) {
        _freeListHead = channel;
    } else {
        _freeListTail->_next = channel;
    }
    _freeListTail = channel;
    _mutex.unlock();

    return true;
}

/*
 * 根据ID，找出一个Channel
 *
 * @param  id: 通道ID
 * @reutrn Channel
 */
Channel *ChannelPool::findChannel(uint32_t id) {
    Channel *channel = NULL;

    __gnu_cxx::hash_map<uint32_t, Channel*>::iterator it;
    _mutex.lock();
    it = _useMap.find(id);
    if (it != _useMap.end()) {
        channel = it->second;
    }
    if (channel) {
        /**@todo why to use such strange logic?*/
        channel->_expireTime += 200000;
    }
    _mutex.unlock();

    return channel;
}

/*
 * 从useList中找出超时的channel的list,并把hashmap中对应的删除
 *
 * @param now: 当前时间
 */
Channel* ChannelPool::getTimeoutList(int64_t now) {
    Channel *list = NULL;

    _mutex.lock();
    if (_useListHead == NULL) { //是空
        _mutex.unlock();
        return list;
    }

    Channel *channel = _useListHead;
    while (channel != NULL) {
        if (channel->_expireTime == 0 || channel->_expireTime >= now) break;
        _useMap.erase(channel->_id);
        channel = channel->_next;
        _useListCount --;
    }

    // 有超时的list
    if (channel != _useListHead) {
        list = _useListHead;
        if (channel == NULL) {  // 全部超时
            _useListHead = _useListTail = NULL;
        } else {
            channel->_prev->_next = NULL;
            channel->_prev = NULL;
            _useListHead = channel;
        }
    }
    _mutex.unlock();

    return list;
}

/*
 * 把addList的链表加入到freeList中
 *
 * @param addList被加的list
 */
bool ChannelPool::appendFreeList(Channel *addList) {
    // 是空
    if (addList == NULL) {
        return true;
    }
    
    _mutex.lock();
    
    // 找tail
    Channel *tail = addList;
    while (tail->_next != NULL) {
        tail = tail->_next;
    }

    // 加入到_freeList
    addList->_prev = _freeListTail;
    if (_freeListTail == NULL) {
        _freeListHead = addList;
    } else {
        _freeListTail->_next = addList;
    }
    _freeListTail = tail;

    _mutex.unlock();
    return true;
}

}


