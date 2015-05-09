#ifndef HTTPCOMPONENTTF_H_
#define HTTPCOMPONENTTF_H_

#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>

namespace anet {
  class HTTPComponentTF : public CppUnit::TestFixture{
  
    CPPUNIT_TEST_SUITE(HTTPComponentTF);
    CPPUNIT_TEST(testCreateConnection);
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();
    void testCreateConnection();
  };
}


#endif /*HTTPCOMPONENTTF_H*/
