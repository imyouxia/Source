#ifndef SYNCHRONOUSTF_H_
#define SYNCHRONOUSTF_H_

#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>
#include <string>

namespace anet {

class SynchronousTestingServer;
class SynchronousTF : public CppUnit::TestFixture {
  
    CPPUNIT_TEST_SUITE(SynchronousTF);
    CPPUNIT_TEST(testSynchronous);
    CPPUNIT_TEST(testSynchronousTimeOut);
    CPPUNIT_TEST(testSynchronousClosedByPeer);
    CPPUNIT_TEST(testSynchronousClosed);
    CPPUNIT_TEST_SUITE_END();

public:
    SynchronousTF();
    ~SynchronousTF();
    void setUp();
    void tearDown();
    void testSynchronous();
    void testSynchronousTimeOut();
    void testSynchronousClosedByPeer();
    void testSynchronousClosed();
private:
    Transport *_transport;
    DefaultPacketFactory _factory;
    DefaultPacketStreamer *_streamer;
    SynchronousTestingServer *_server;
    std::string _spec;
    IOComponent *_listener;
    Connection *_connection;
    DefaultPacket *_packet;
    ControlPacket *_cmd;
};
}


#endif /*SYNCHRONOUSTF_H*/
