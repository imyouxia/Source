#ifndef HTTPREQUESTANDRESPONSEPACKETTF_H_
#define HTTPREQUESTANDRESPONSEPACKETTF_H_

#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>
#include <anet/httpresponsepacket.h>
#include <anet/httprequestpacket.h>

namespace anet {
class HttpRequestAndResponsePacketTF : public CppUnit::TestFixture{
  
    CPPUNIT_TEST_SUITE(HttpRequestAndResponsePacketTF);
    CPPUNIT_TEST(testDecodeAndEncode);
    CPPUNIT_TEST(testSetBody);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();
    void testDecodeAndEncode();
    void testSetBody();
    HttpResponsePacket* doReply(HttpRequestPacket *);
};
}


#endif /*HTTPREQUESTANDRESPONSEPACKETTF_H_*/
