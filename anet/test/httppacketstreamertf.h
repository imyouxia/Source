#ifndef HTTPPACKETSTREAMERTF_H_
#define HTTPPACKETSTREAMERTF_H__

#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>

namespace anet {
  class HttpPacketStreamerTF : public CppUnit::TestFixture{
  
    CPPUNIT_TEST_SUITE(HttpPacketStreamerTF);
    CPPUNIT_TEST(testGetPacketInfo);
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();
    void testGetPacketInfo();
  };
}


#endif /*HTTPPACKETSTREAMERTF_H_*/
