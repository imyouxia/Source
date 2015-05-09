/**
 * $Id: packetqueuetf.h 15405 2008-12-12 10:02:04Z zhangli $	 
 */
   
#ifndef ANET__H_
#define PACKETQUEUETF_H_
#include <cppunit/extensions/HelperMacros.h>

namespace anet {
class PacketQueueTF : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(PacketQueueTF);
    CPPUNIT_TEST(testPush);
    CPPUNIT_TEST(testPop); 
    CPPUNIT_TEST(testMoveTo); 
    CPPUNIT_TEST(testGetTimeoutList);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();
    void testPush();
    void testPop();
    void testMoveTo();
    void testGetTimeoutList();
};
}

#endif /*PACKETQUEUETF_H_*/
