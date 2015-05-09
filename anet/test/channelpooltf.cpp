#include <anet/log.h>
#include "channelpooltf.h"
#include <unistd.h>
#include <anet/channel.h>

using namespace std;

namespace anet {
    
CPPUNIT_TEST_SUITE_REGISTRATION(CHANNELPOOLTF);
	
void CHANNELPOOLTF::setUp() {
    threadCount = 5;
}

void CHANNELPOOLTF::tearDown() {
}

size_t CHANNELPOOLTF::getChannelListSize(Channel *head) {
    if (head == NULL) return 0;
    size_t size = 0;
    while (head) {
        size ++;
        head = head->_next;
    }
    return size;
}

size_t CHANNELPOOLTF::getChannelListSizeByTail(Channel *tail) {
    if (tail == NULL) return 0;
    size_t size = 0;
    while (tail) {
        size ++;
        tail = tail->_prev;
    }
    return size;
}

void CHANNELPOOLTF::testAllocateWhenExceedMax() {
    ANET_LOG(DEBUG, "BEGIN testAllocateWhenExceedMax()");
    _channelPool = new ChannelPool;
    atomic_set(&ChannelPool::_globalChannelId, 
               ChannelPool::AUTO_CHANNEL_ID_MAX);
    
    Channel *channel1 = _channelPool->allocChannel(0);
    Channel *channel2 = _channelPool->allocChannel(0);

    CPPUNIT_ASSERT_EQUAL(1u, channel1->getId());
    CPPUNIT_ASSERT_EQUAL(2u, channel2->getId());
    _channelPool->freeChannel(channel1);
    _channelPool->freeChannel(channel2);
    delete _channelPool;
    ANET_LOG(DEBUG,"END testAllocateWhenExceedMax()");
}

void CHANNELPOOLTF::testAllocChannel() {
    ANET_LOG(DEBUG, "BEGIN testAllocChannel() _globalChannelId: |%d|", ChannelPool::_globalChannelId);
    atomic_set(&ChannelPool::_globalChannelId, 0);
    _channelPool = new ChannelPool;
    Channel *channel = NULL;
    channel = _channelPool->allocChannel(0);
    CPPUNIT_ASSERT(channel);
    CPPUNIT_ASSERT(!channel->getHandler());
    //test _clusterList
    CPPUNIT_ASSERT_EQUAL((size_t)1, _channelPool->_clusterList.size());
    //test _useList
    CPPUNIT_ASSERT_EQUAL((size_t)1, getChannelListSize(_channelPool->_useListHead));
    CPPUNIT_ASSERT_EQUAL(channel, _channelPool->_useListHead);
    CPPUNIT_ASSERT_EQUAL(_channelPool->_useListTail, _channelPool->_useListHead);
    //test _freeList_
    CPPUNIT_ASSERT_EQUAL(CHANNEL_CLUSTER_SIZE-1, getChannelListSize(_channelPool->_freeListHead));
    //test getUseListCount()
    CPPUNIT_ASSERT_EQUAL((size_t)1, _channelPool->getUseListCount());
    //test _useMap
    CPPUNIT_ASSERT_EQUAL(_channelPool->_useMap[1], channel);

    channel = _channelPool->allocChannel(ChannelPool::AUTO_CHANNEL_ID_MAX - 1);
    CPPUNIT_ASSERT(channel);
    CPPUNIT_ASSERT_EQUAL(2u, channel->_id);
    CPPUNIT_ASSERT_EQUAL(_channelPool->_useMap[2], channel);
    channel = _channelPool->allocChannel(ChannelPool::AUTO_CHANNEL_ID_MAX);
    CPPUNIT_ASSERT(channel);
    CPPUNIT_ASSERT_EQUAL(3u, channel->_id);
    CPPUNIT_ASSERT_EQUAL(_channelPool->_useMap[3], channel);
    channel = _channelPool->allocChannel(ChannelPool::AUTO_CHANNEL_ID_MAX + 1);
    CPPUNIT_ASSERT(channel);
    CPPUNIT_ASSERT_EQUAL(ChannelPool::AUTO_CHANNEL_ID_MAX+1, channel->_id);
    CPPUNIT_ASSERT_EQUAL(_channelPool->_useMap[ChannelPool::AUTO_CHANNEL_ID_MAX + 1], channel);	
	
    for(size_t i = 0; i < CHANNEL_CLUSTER_SIZE-4; i++) {
        _channelPool->allocChannel(0);
    }
    CPPUNIT_ASSERT_EQUAL(CHANNEL_CLUSTER_SIZE, getChannelListSize(_channelPool->_useListHead));
    CPPUNIT_ASSERT_EQUAL((size_t)0, getChannelListSize(_channelPool->_freeListHead));
    CPPUNIT_ASSERT_EQUAL(getChannelListSize(_channelPool->_useListHead), _channelPool->getUseListCount());

    channel = _channelPool->allocChannel(0);
    CPPUNIT_ASSERT(channel);
    CPPUNIT_ASSERT_EQUAL(CHANNEL_CLUSTER_SIZE, (size_t)channel->_id);
    CPPUNIT_ASSERT_EQUAL(CHANNEL_CLUSTER_SIZE+(size_t)1, getChannelListSize(_channelPool->_useListHead));
    CPPUNIT_ASSERT_EQUAL(CHANNEL_CLUSTER_SIZE-(size_t)1, getChannelListSize(_channelPool->_freeListHead));
    CPPUNIT_ASSERT_EQUAL(2,(int)_channelPool->_clusterList.size());
    CPPUNIT_ASSERT_EQUAL(getChannelListSize(_channelPool->_useListHead), _channelPool->getUseListCount());
    CPPUNIT_ASSERT_EQUAL(_channelPool->_useMap[channel->_id], channel);
	

    delete _channelPool;
    ANET_LOG(DEBUG,"END testAllocChannel()");
}

void CHANNELPOOLTF::testFreeChannel() {
    ANET_LOG(DEBUG, "BEGIN testFreeChannel()");
    _channelPool = new ChannelPool;

    /*
     *  test parameter is null
     */
    CPPUNIT_ASSERT(!_channelPool->freeChannel(NULL));

    /*
     *  only one channel in useList
     */
    Channel *channel = _channelPool->allocChannel(0);
    CPPUNIT_ASSERT(_channelPool->freeChannel(channel));//check return value
    CPPUNIT_ASSERT_EQUAL((size_t)0, _channelPool->_useMap.size());//check _useMap
    CPPUNIT_ASSERT(!_channelPool->_useListHead);//check _useList
    CPPUNIT_ASSERT(!_channelPool->_useListTail);
    CPPUNIT_ASSERT_EQUAL(_channelPool->_useMap.size(), _channelPool->_useListCount);//check _useListCount
    CPPUNIT_ASSERT_EQUAL(channel, _channelPool->_freeListTail);//check _freeList
    delete _channelPool;

    /*
     * normal situation
     */
    _channelPool = new ChannelPool;
    Channel *head = _channelPool->allocChannel(0);
    CPPUNIT_ASSERT(_channelPool->allocChannel(0));
    CPPUNIT_ASSERT(_channelPool->allocChannel(0));
    Channel *body = _channelPool->allocChannel(0);
    CPPUNIT_ASSERT(_channelPool->allocChannel(0));
    CPPUNIT_ASSERT(_channelPool->allocChannel(0));
    Channel *tail = _channelPool->allocChannel(0);

    //free head
    Channel *tmpUseListHeadNext = _channelPool->_useListHead->_next;
    CPPUNIT_ASSERT(_channelPool->freeChannel(head));//check return value
    CPPUNIT_ASSERT_EQUAL(6, (int)_channelPool->_useMap.size());//check _useMap
    CPPUNIT_ASSERT_EQUAL(tmpUseListHeadNext, _channelPool->_useListHead);//check _useList head
    CPPUNIT_ASSERT_EQUAL(_channelPool->_useMap.size(), _channelPool->_useListCount);//check _useListCount
    CPPUNIT_ASSERT_EQUAL(head, _channelPool->_freeListTail); //check _freeList
    CPPUNIT_ASSERT(!head->_next);

    //free body
    Channel *tmpFreeListTail = _channelPool->_freeListTail;
    Channel *tmpBodyPrev = body->_prev;
    Channel *tmpBodyNext = body->_next;
    CPPUNIT_ASSERT(_channelPool->freeChannel(body));//check return value
    CPPUNIT_ASSERT_EQUAL(5, (int)_channelPool->_useMap.size());//check _useMap
    CPPUNIT_ASSERT_EQUAL(tmpBodyPrev->_next, tmpBodyNext);//check _useList
    CPPUNIT_ASSERT_EQUAL(tmpBodyNext->_prev, tmpBodyPrev);
    CPPUNIT_ASSERT_EQUAL(_channelPool->_useMap.size(), _channelPool->_useListCount);//check _useListCount
    CPPUNIT_ASSERT_EQUAL(body, _channelPool->_freeListTail);//check _freeList
    CPPUNIT_ASSERT_EQUAL(tmpFreeListTail, body->_prev);
    CPPUNIT_ASSERT(!body->_next);

    //free tail
    Channel *tmpUseListTailPrev = _channelPool->_useListTail->_prev;
    tmpFreeListTail = _channelPool->_freeListTail;
    CPPUNIT_ASSERT(_channelPool->freeChannel(tail));//check return value
    CPPUNIT_ASSERT_EQUAL(4, (int)_channelPool->_useMap.size());//check _useMap
    CPPUNIT_ASSERT_EQUAL(_channelPool->_useListTail, tmpUseListTailPrev);//check _useList
    CPPUNIT_ASSERT(_channelPool->_useListTail);
    CPPUNIT_ASSERT_EQUAL(_channelPool->_useMap.size(), _channelPool->_useListCount);//check _useListCount
    CPPUNIT_ASSERT_EQUAL(tail, _channelPool->_freeListTail);//check _freeList
    CPPUNIT_ASSERT_EQUAL(tmpFreeListTail, tail->_prev);
    CPPUNIT_ASSERT(!tail->_next);
	
    delete _channelPool;

    /*
     * check _freeListTail is null
     */
    _channelPool = new ChannelPool;
    for(size_t i = 0; i < CHANNEL_CLUSTER_SIZE-1; i++) {
        _channelPool->allocChannel(0);
    }
    channel = _channelPool->allocChannel(0);
    CPPUNIT_ASSERT(!_channelPool->_freeListTail);
    CPPUNIT_ASSERT(_channelPool->freeChannel(channel));//check return value
    CPPUNIT_ASSERT_EQUAL(_channelPool->_freeListHead, _channelPool->_freeListTail);
    CPPUNIT_ASSERT_EQUAL(_channelPool->_freeListHead, channel);
	
    delete _channelPool;
    ANET_LOG(DEBUG,"END testFreeChannel()");
}

void CHANNELPOOLTF::testFindChannel() {
    _channelPool = new ChannelPool;
    CPPUNIT_ASSERT(!_channelPool->findChannel((uint32_t)0));
    delete _channelPool;
		
    //find the head body and tail
    _channelPool = new ChannelPool;
    Channel *head = _channelPool->allocChannel(0);
    CPPUNIT_ASSERT(head);
    _channelPool->allocChannel(0);
    _channelPool->allocChannel(0);
    Channel *body = _channelPool->allocChannel(0);
    CPPUNIT_ASSERT(body);
    _channelPool->allocChannel(0);
    _channelPool->allocChannel(0);
    Channel *tail = _channelPool->allocChannel(0);
    int64_t headExpireTime = head->_expireTime;
    int64_t bodyExpireTime = body->_expireTime;
    int64_t tailExpireTime = tail->_expireTime;

    //~@todo check if return value is correct
    CPPUNIT_ASSERT(_channelPool->findChannel(head->getId()));
    CPPUNIT_ASSERT(_channelPool->findChannel(body->getId()));
    CPPUNIT_ASSERT(_channelPool->findChannel(tail->getId()));
    CPPUNIT_ASSERT_EQUAL((int64_t)200000, head->_expireTime - headExpireTime);
    CPPUNIT_ASSERT_EQUAL((int64_t)200000, body->_expireTime - bodyExpireTime);
    CPPUNIT_ASSERT_EQUAL((int64_t)200000, tail->_expireTime - tailExpireTime);
    delete _channelPool;
}

void CHANNELPOOLTF::testGetTimeoutList() {
    _channelPool = new ChannelPool;
    //NULL
    CPPUNIT_ASSERT(!_channelPool->getTimeoutList(TimeUtil::getTime()));
    //no time out channelpp
    Channel *head = _channelPool->allocChannel(0);
    Channel *body = _channelPool->allocChannel(0);
    Channel *tail = _channelPool->allocChannel(0);
    CPPUNIT_ASSERT(head);
    CPPUNIT_ASSERT(body);
    CPPUNIT_ASSERT(tail);

    CPPUNIT_ASSERT(!_channelPool->getTimeoutList(TimeUtil::getTime()));

    int64_t now = TimeUtil::getTime();		

    //only the head time out
    head->setExpireTime(now);
    Channel *tmpHeadNext = head->_next;
    CPPUNIT_ASSERT(_channelPool->getTimeoutList(now+1));
    CPPUNIT_ASSERT_EQUAL(tmpHeadNext, _channelPool->_useListHead);
    //normal situation
    head = body;
    body = tail;
    tail = _channelPool->allocChannel(0);
    head->setExpireTime(now+100);
    body->setExpireTime(now+200);
    Channel *tmpAssumeHead = head->_next->_next;
    CPPUNIT_ASSERT(_channelPool->getTimeoutList(now+201));
    CPPUNIT_ASSERT_EQUAL(tmpAssumeHead, _channelPool->_useListHead);
    //all the list time out
    head = tail;
    body = _channelPool->allocChannel(0);
    tail = _channelPool->allocChannel(0);
    head->setExpireTime(now+100);
    body->setExpireTime(now+200);
    tail->setExpireTime(now+300);
    CPPUNIT_ASSERT(_channelPool->getTimeoutList(now+301));
    CPPUNIT_ASSERT(!_channelPool->_useListHead);
    delete _channelPool;
}

void CHANNELPOOLTF::testAppendFreeList() {
    //append NULL
    _channelPool = new ChannelPool;
    size_t tmpFreeListLength = getChannelListSize(_channelPool->_freeListHead);
    CPPUNIT_ASSERT(_channelPool->appendFreeList(NULL));
    CPPUNIT_ASSERT_EQUAL(tmpFreeListLength, getChannelListSize(_channelPool->_freeListHead));
    int64_t now = TimeUtil::getTime();
    for(int i = 0; i < 5; i++) {
        Channel *channel;
        channel = _channelPool->allocChannel(0);
        channel->setExpireTime(now+20);
    }
    Channel *timeoutList = _channelPool->getTimeoutList(now+21);
    CPPUNIT_ASSERT(_channelPool->appendFreeList(timeoutList));
	
    ChannelPool *anotherPool = new ChannelPool;
    tmpFreeListLength = getChannelListSize(anotherPool->_freeListHead);
    CPPUNIT_ASSERT(anotherPool->appendFreeList(timeoutList));
    CPPUNIT_ASSERT_EQUAL(tmpFreeListLength+5, getChannelListSize(anotherPool->_freeListHead));
	

    delete anotherPool;
    delete _channelPool;
}
}
