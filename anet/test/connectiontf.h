/**
 * $Id: connectiontf.h 15756 2008-12-30 06:58:52Z zhangli $
 */
   
#ifndef CONNECTIONTF_H_
#define CONNECTIONTF_H_
#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>

namespace anet {

class ConnectionTF : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ConnectionTF);
    CPPUNIT_TEST(testPostPacket);
    CPPUNIT_TEST(testHandlePacket);
    CPPUNIT_TEST(testCheckTimeout);
    CPPUNIT_TEST(testCloseHook);
    CPPUNIT_TEST(testClose);
    CPPUNIT_TEST(testTimeoutMultiThreadSafe);
    CPPUNIT_TEST(testHTTPHandler);
    CPPUNIT_TEST(testConstructorNoHeader);
    CPPUNIT_TEST(testHTTPTimeoutBeforeSend);
    CPPUNIT_TEST(testHTTPTimeoutAfterSend);
    CPPUNIT_TEST(testGetPacketPostedCount);
    CPPUNIT_TEST(testGetPacketHandledCount);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();
    void testPostPacket();
    void testHandlePacket();
    void testCheckTimeout();
    void testCloseHook();
    void testClose();
    void testTimeoutMultiThreadSafe();
    void testHTTPHandler();
    void testConstructorNoHeader();
    void testHTTPTimeoutBeforeSend();
    void testHTTPTimeoutAfterSend();
    void testGetPacketPostedCount();
    void testGetPacketHandledCount();
     ConnectionTF();
    ~ConnectionTF();
private:
    Connection *_connection;
    IOComponent *_component;
    IPacketStreamer *_streamer;
    IPacketFactory *_factory;
    IPacketHandler *_handler;
    IServerAdapter *_serverAdapter;
};

class HTTPHandler : public IPacketHandler, public IServerAdapter {
public:
    HTTPHandler() : requestCount(0), responseCount(0) {
        for (int i = 0; i < ControlPacket::CMD_END; i ++) {
            clientControlCount[i] = 0;
            serverControlCount[i] = 0;
        }
    }
    IPacketHandler::HPRetCode handlePacket(Packet *packet, void *args);
    IPacketHandler::HPRetCode 
    handlePacket(Connection *connection, Packet *packet);
public:
    int requestCount;
    int responseCount;
    int serverControlCount[ControlPacket::CMD_END];
    int clientControlCount[ControlPacket::CMD_END];
};

}
#endif /*CONNECTIONTF_H_*/
