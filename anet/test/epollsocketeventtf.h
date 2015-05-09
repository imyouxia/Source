/**
 * $Id: epollsocketeventtf.h 15845 2009-01-04 09:33:25Z zhangli $
 */
   
#ifndef EPOLLSOCKETEVENTTF_H_
#define EPOLLSOCKETEVENTTF_H_
#include <cppunit/extensions/HelperMacros.h>
#include <anet/serversocket.h>

namespace anet {
class EPollSocketEventTF : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(EPollSocketEventTF);
    CPPUNIT_TEST(testAddEvent);
    CPPUNIT_TEST(testEmptyEvent);
    CPPUNIT_TEST(testSetEvent);
    CPPUNIT_TEST(testRemoveEvent);
    CPPUNIT_TEST(testClose);
    CPPUNIT_TEST(testWakeUp);
    CPPUNIT_TEST(testDup);
    CPPUNIT_TEST_SUITE_END();
public:
    EPollSocketEventTF();
    void setUp();
    void tearDown();
    void testAddEvent();
    void testEmptyEvent();
    void testSetEvent();
    void testRemoveEvent();
    void testClose();
    void testWakeUp();
    void testDup();
protected:
    Socket _tcpClient;
    ServerSocket _tcpListener;
    Socket *_tcpAccepted;
    Socket udpSocket1;
    Socket udpSocket2;
    
    IOComponent *_dummyClient;
    IOComponent *_dummyListener;
    IOComponent *_dummyAccepted;
};
}

#endif /*EPOLLSOCKETEVENTTF_H_*/
