#ifndef ANET_STREAMINGCONTEXTTF_H_
#define ANET_STREAMINGCONTEXTTF_H_

#include <cppunit/extensions/HelperMacros.h>
#include <anet/streamingcontext.h>

namespace anet {
class StreamingContextTF : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(StreamingContextTF);
    CPPUNIT_TEST(testIsAndSetCompleted);
    CPPUNIT_TEST(testIsAndSetBroken);
    CPPUNIT_TEST(testIsAndSetEndOfFile);
    CPPUNIT_TEST(testGetAndSetPacket);
    CPPUNIT_TEST(testStealPacket);
    CPPUNIT_TEST(reset);
    CPPUNIT_TEST(testDestructor);
    CPPUNIT_TEST(testSetGetErrorNo);
    CPPUNIT_TEST(testSetGetErrorString);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();    
    void testIsAndSetCompleted();
    void testIsAndSetBroken();
    void testIsAndSetEndOfFile();
    void testGetAndSetPacket();
    void testStealPacket();
    void reset();
    void testDestructor(); 
    void testSetGetErrorNo();
    void testSetGetErrorString();
    StreamingContext *_context;
};

}/*end namespace anet*/
#endif/* ANET_STREAMINGCONTEXTTF_H_*/
