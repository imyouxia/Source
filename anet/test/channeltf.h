/**
 * $Id: channeltf.h 15845 2009-01-04 09:33:25Z zhangli $	 
 */
   
#ifndef CHANNELTF_H_
#define CHANNELTF_H_
#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>
#include <anet/channel.h>

namespace anet {
class CHANNELTF : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(CHANNELTF);
    CPPUNIT_TEST(testGetId);
    CPPUNIT_TEST(testGetArgs);
    CPPUNIT_TEST(testSetHandler);
    CPPUNIT_TEST(testGetHandler);
    CPPUNIT_TEST(testSetExpireTime);
    CPPUNIT_TEST(testGetNext);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();
    void testGetId();
    void testGetArgs();
    void testSetHandler();
    void testGetHandler();
    void testSetExpireTime();
    void testGetNext();

private:
    Channel _channel;
};
}

#endif /*CHANNELTF_H_*/
