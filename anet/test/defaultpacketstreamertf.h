/**
 * defaultpacketstreamertf.h 2008-8-6 hua.huangrh
 */

#ifndef DEFAULTPACKETSTREAMERTF_H_
#define DEFAULTPACKETSTREAMERTF_H_

#define DATA_MAX_SIZE 1024

#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>

namespace anet{
class MyPacketFactory;
class DEFAULTPACKETSTREAMERTF : public CppUnit::TestFixture{
    CPPUNIT_TEST_SUITE(DEFAULTPACKETSTREAMERTF);
    CPPUNIT_TEST(testGetPacketInfo);
    CPPUNIT_TEST(testDecode);
    CPPUNIT_TEST(testEncode);
    CPPUNIT_TEST(testProcessData);
    CPPUNIT_TEST(testProcessDataMultiStep);
    CPPUNIT_TEST(testProcessDataErrorHttpRequestPacket);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();
    void testGetPacketInfo();
    void testDecode();
    void testEncode();
    void testProcessData();
    void testProcessDataMultiStep();
    void testProcessDataErrorHttpRequestPacket();
private:
    MyPacketFactory *_factory;
};
}

#endif /* DEFAULTPACKETSTREAMERTF_H_ */
