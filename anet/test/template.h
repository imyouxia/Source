#ifndef TEMPLATETF_H_
#define TEMPLATETF_H_

#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>

namespace anet {
class TemplateTF : public CppUnit::TestFixture {
  
    CPPUNIT_TEST_SUITE(TemplateTF);
    CPPUNIT_TEST(testTemplate);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();
    void testTemplate();
};
}


#endif /*TEMPLATETF_H*/
