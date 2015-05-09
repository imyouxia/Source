#ifndef ANET_HTTPPACKETTF_H_
#define ANET_HTTPPACKETTF_H_

#include <cppunit/extensions/HelperMacros.h>
#include <anet/httppacket.h>
#include <anet/httpstreamer.h>
#include <anet/httppacketfactory.h>

namespace anet {
class HTTPPacketTF : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(HTTPPacketTF);
    CPPUNIT_TEST(testSetGetPacketType);
    CPPUNIT_TEST(testSetGetVersion);
    CPPUNIT_TEST(testSetGetMethod);
    CPPUNIT_TEST(testSideEffect);
    CPPUNIT_TEST(testSetGetURI);
    CPPUNIT_TEST(testSetGetStatusCode);
    CPPUNIT_TEST(testSetGetReasonPhrase);
    CPPUNIT_TEST(testAddGetHeader);
    CPPUNIT_TEST(testGetHeaderMemCopy);
    CPPUNIT_TEST(testInitialPacket);
    CPPUNIT_TEST(testEncodeNoType);
    CPPUNIT_TEST(testEncodeRequestNoMethod);
    CPPUNIT_TEST(testEncodeRequestNoURI);
    CPPUNIT_TEST(testEncodeGet);
    CPPUNIT_TEST(testEncodeMethods);
    CPPUNIT_TEST(testEncodeURI);
    CPPUNIT_TEST(testEncodeVersion);
    CPPUNIT_TEST(testEncodeBody);
    CPPUNIT_TEST(testResponseNoStatusCode);
    CPPUNIT_TEST(testResponseNoReasonPhrase);
    CPPUNIT_TEST(testSetResponseVersion);
    CPPUNIT_TEST(testResponseEncode);
    CPPUNIT_TEST(testKeepAlive);
    CPPUNIT_TEST(testKeepAliveVersion);
    CPPUNIT_TEST(testSetKeepAlive);
    CPPUNIT_TEST(testHeaderIteration);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();
    void testSetGetPacketType();
    void testSetGetVersion();
    void testSetGetMethod();
    void testSideEffect();
    void testSetGetURI();
    void testSetGetStatusCode();
    void testSetGetReasonPhrase();
    void testAddGetHeader();
    void testGetHeaderMemCopy();
    void testInitialPacket();
    void testEncodeNoType();
    void testEncodeRequestNoMethod();
    void testEncodeRequestNoURI();
    void testEncodeGet();
    void testEncodeMethods();
    void testEncodeURI() ;
    void testEncodeVersion();
    void testEncodeBody();
    void testResponseNoStatusCode();
    void testResponseNoReasonPhrase();
    void testSetResponseVersion();
    void testResponseEncode();
    void testKeepAlive();
    void testKeepAliveVersion();
    void testSetKeepAlive();
    void testHeaderIteration();
    HTTPPacket *_packet;
    HTTPPacketFactory *_factory;
    HTTPStreamer *_streamer;
    HTTPStreamingContext *_context;
    DataBuffer *_dataBuffer;
};

}/*end namespace anet*/
#endif/* ANET_HTTPPACKETTF_H_*/
