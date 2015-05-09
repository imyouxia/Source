#ifndef ANET_HTTPSTREAMINGCONTEXTTF_H_
#define ANET_HTTPSTREAMINGCONTEXTTF_H_

#include <cppunit/extensions/HelperMacros.h>

namespace anet {
class HTTPStreamingContextTF : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(HTTPStreamingContextTF);
    CPPUNIT_TEST(testHTTPStreamingContext);
    CPPUNIT_TEST(testSetGetErrorNo);
    CPPUNIT_TEST(testSetGetErrorString);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();
    void testHTTPStreamingContext();
    void testSetGetErrorNo();
    void testSetGetErrorString();
};

}/*end namespace anet*/
#endif/* ANET_HTTPSTREAMINGCONTEXTTF_H_*/
