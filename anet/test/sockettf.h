/**
 * $Id: sockettf.h 15405 2008-12-12 10:02:04Z zhangli $	 
 */
   
#ifndef SOCKETTF_H_
#define SOCKETTF_H_
#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>

namespace anet {
class SocketTF : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(SocketTF);
    CPPUNIT_TEST(testSetGetAddress); // you can register more methods here
    CPPUNIT_TEST(testReadWrite); // you can register more methods here
    CPPUNIT_TEST(testConnect);
    CPPUNIT_TEST(testListenZeroIPZeroPort);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();
    void testSetGetAddress();
    void testReadWrite();
    void testConnect();
    void testListenZeroIPZeroPort();
};
}

#endif /*SOCKETTF_H_*/
