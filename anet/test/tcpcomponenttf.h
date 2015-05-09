/**
 * p$Id: tcpcomponenttf.h 15689 2008-12-26 13:27:52Z zhangli $
 */
   
#ifndef TCPCOMPONENTTF_H_
#define TCPCOMPONENTTF_H_
#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>
#include <anet/defaultpacketfactory.h>

namespace anet {
class TCPCOMPONENTTF : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TCPCOMPONENTTF);
    CPPUNIT_TEST(testSocketConnectAndClose);
    CPPUNIT_TEST(testHandleWriteEvent);
    CPPUNIT_TEST(testHandleWriteEventReconnect);
    CPPUNIT_TEST(testHandleReadEvent);
    CPPUNIT_TEST(testHandleErrorEventConnecting);
    CPPUNIT_TEST(testHandleErrorEventConnected);
    CPPUNIT_TEST(testInit);
    CPPUNIT_TEST(testCreateConnection);
    CPPUNIT_TEST(testCheckTimeout);
    CPPUNIT_TEST(testIdleTime);
    CPPUNIT_TEST(testCloseSocketNoLock);
    CPPUNIT_TEST_SUITE_END();
    
public:
    TCPCOMPONENTTF();
    ~TCPCOMPONENTTF();
    void setUp();
    void tearDown();
    void testSocketConnectAndClose();
    void testHandleWriteEvent();
    void testHandleWriteEventReconnect();
    void testHandleReadEvent();
    void testHandleErrorEventConnecting();
    void testHandleErrorEventConnected();
    void testInit();
    void testCreateConnection();
    void testCheckTimeout();
    void testIdleTime();
    void testCloseSocketNoLock();
    int getListCountFromHead(IOComponent *);
private:
    DefaultPacketStreamer *_streamer;
    DefaultPacketFactory *_factory;
};
}

#endif /*TCPCOMPONENTTF_H_*/
