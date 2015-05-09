/**
 * $Id: channelpooltf.h 15689 2008-12-26 13:27:52Z zhangli $
 */
   
#ifndef CHANNELPOOLTF_H_
#define CHANNELPOOLTF_H_
#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>

namespace anet {
class CHANNELPOOLTF : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(CHANNELPOOLTF);
    CPPUNIT_TEST(testAllocChannel);
    CPPUNIT_TEST(testFreeChannel);
    CPPUNIT_TEST(testFindChannel);
    CPPUNIT_TEST(testGetTimeoutList);
    CPPUNIT_TEST(testAppendFreeList);
//    CPPUNIT_TEST(testAllocateWhenExceedMax);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();
    void testAllocChannel();
    void testFreeChannel();
    void testFindChannel();
    void testGetTimeoutList();
    void testAppendFreeList();
    void testAllocateWhenExceedMax();

private:
    ChannelPool *_channelPool;
    int threadCount;
    size_t getChannelListSize(Channel *);
    size_t getChannelListSizeByTail(Channel *);
};
}

#endif /*CHANNELPOOLTF_H_*/
