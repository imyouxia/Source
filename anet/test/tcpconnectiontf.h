/**
 * $Id: tcpconnectiontf.h 15849 2009-01-04 12:09:57Z hua.huangrh $
 */
   
#ifndef TCPCONNECTIONTF_H_
#define TCPCONNECTIONTF_H_
#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>
#include <anet/serversocket.h>
#include <anet/tcpconnection.h>

namespace anet {
class ConnPacketFactory;
class DefaultPacketStreamer;
class DefaultPacketHandler;
  
class TCPCONNECTIONTF : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TCPCONNECTIONTF);
    CPPUNIT_TEST(testLock);
    CPPUNIT_TEST(testWriteData);
    CPPUNIT_TEST(testReadData);
    CPPUNIT_TEST(testClose);
    CPPUNIT_TEST(testMemLeak);
    CPPUNIT_TEST(testReadMaliciousDataTooLarge);
    CPPUNIT_TEST(testReadMaliciousDataInfinitLoop);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();
    void testWriteData();
    void testReadData();
    void testClose();
    void testLock();
    void testMemLeak();
    void testReadMaliciousDataTooLarge();
    void testReadMaliciousDataInfinitLoop();

    TCPCONNECTIONTF();
    ~TCPCONNECTIONTF();
private:
    int getListCountFromHead(IOComponent *);
    ConnPacketFactory *_factory;
    DefaultPacketStreamer *_streamer;
    DefaultPacketHandler *_handler;
    
    Transport *_transport;
    ServerSocket *_server;
    Socket *_accept;
    TCPConnection *_conn;
};
}

#endif /*TCPCONNECTIONTF_H_*/
