#include <anet/anet.h>
#include "packetqueuetf.h"
#include <iostream>

using namespace std;

namespace anet {
    
CPPUNIT_TEST_SUITE_REGISTRATION(PacketQueueTF);

void PacketQueueTF::setUp() {
}

void PacketQueueTF::tearDown() {
}

void PacketQueueTF::testPush() {
    PacketQueue packetQueue;
    
    for(size_t i=0; i<4; i++) {
        Packet *packet = new ControlPacket(0);
        packetQueue.push(packet);
        CPPUNIT_ASSERT_EQUAL(i+1, packetQueue.size());
    }
    for(size_t i=0; i<4; i++) {
        Packet *packet = packetQueue.pop();
        CPPUNIT_ASSERT(packet != NULL);
        delete packet;
        CPPUNIT_ASSERT_EQUAL(3-i, packetQueue.size());
    }
}

void PacketQueueTF::testPop() {
}

void PacketQueueTF::testMoveTo() {
    PacketQueue packetQueue1;
    PacketQueue packetQueue2;
    
    // null => null
    packetQueue1.moveTo(&packetQueue2);
    CPPUNIT_ASSERT_EQUAL((size_t)0, packetQueue1.size());
    CPPUNIT_ASSERT_EQUAL((size_t)0, packetQueue2.size());
    
    // 1 个 => null
    packetQueue1.push(new ControlPacket(0));
    packetQueue1.moveTo(&packetQueue2);
    CPPUNIT_ASSERT_EQUAL((size_t)0, packetQueue1.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, packetQueue2.size());
    
    // null => 1个
    packetQueue1.moveTo(&packetQueue2);
    CPPUNIT_ASSERT_EQUAL((size_t)0, packetQueue1.size());
    CPPUNIT_ASSERT_EQUAL((size_t)1, packetQueue2.size());
    
    // 1个 => 1个
    packetQueue1.push(new ControlPacket(0));
    packetQueue1.moveTo(&packetQueue2);
    CPPUNIT_ASSERT_EQUAL((size_t)0, packetQueue1.size());
    CPPUNIT_ASSERT_EQUAL((size_t)2, packetQueue2.size());
    
    // pop 两个
    for(size_t i=0; i<2; i++) {
        Packet *packet = packetQueue2.pop();
        CPPUNIT_ASSERT(packet != NULL);
        delete packet;
        CPPUNIT_ASSERT_EQUAL(1-i, packetQueue2.size());
    }
}

void PacketQueueTF::testGetTimeoutList() {
    class DummyPacket : public Packet {
    public:
        bool encode(DataBuffer*) {return true;}
        bool decode(DataBuffer*, PacketHeader*) {return true;}
    };

    PacketQueue packetQueue;
    Packet *packet1, *packet2, *timeoutList;

    CPPUNIT_ASSERT(! packetQueue.getTimeoutList(TimeUtil::MIN));
    CPPUNIT_ASSERT(packet1 = new DummyPacket);
    CPPUNIT_ASSERT(packet2 = new DummyPacket);

    int64_t now  = TimeUtil::getTime();    
    packet1->setExpireTime(100);
    packet2->setExpireTime(500);
    //one packet
    packetQueue.push(packet1);
    CPPUNIT_ASSERT(!packetQueue.getTimeoutList(now));
    CPPUNIT_ASSERT(timeoutList = packetQueue.getTimeoutList(now + 300000));
    CPPUNIT_ASSERT_EQUAL(packet1, timeoutList);
    CPPUNIT_ASSERT_EQUAL((size_t)0, packetQueue.size());
    CPPUNIT_ASSERT(!packetQueue.pop());
    packetQueue.push(timeoutList);
    CPPUNIT_ASSERT_EQUAL(packet1, packetQueue.pop());

    packetQueue.push(packet1);
    packetQueue.push(packet2);
    CPPUNIT_ASSERT(!packetQueue.getTimeoutList(now));
    CPPUNIT_ASSERT(timeoutList = packetQueue.getTimeoutList(now + 300000));
    CPPUNIT_ASSERT_EQUAL((size_t)1, packetQueue.size()); 
    packetQueue.push(packet1);
    CPPUNIT_ASSERT_EQUAL(packet2, packetQueue.pop());    
    CPPUNIT_ASSERT_EQUAL(packet1, packetQueue.pop());    

    packetQueue.push(packet1);
    packetQueue.push(packet2);
    CPPUNIT_ASSERT(timeoutList = packetQueue.getTimeoutList(now + 600000));
    CPPUNIT_ASSERT_EQUAL((size_t)0, packetQueue.size());
    packet1->free();
    packet2->free();
}

}
