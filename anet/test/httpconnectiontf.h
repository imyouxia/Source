#ifndef ANET_HTTPCONNECTIONTF_H_
#define ANET_HTTPCONNECTIONTF_H_

#include <cppunit/extensions/HelperMacros.h>

namespace anet {
class HttpConnectionTF : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(HttpConnectionTF);
    CPPUNIT_TEST(testHttpConnection);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();
    void testHttpConnection();
};

}/*end namespace anet*/
#endif/* ANET_HTTPCONNECTIONTF_H_*/
