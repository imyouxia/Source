/**
 * $Id: controlpackettf.h 15405 2008-12-12 10:02:04Z zhangli $	 
 */
   
#ifndef CONTROLPACKETTF_H_
#define CONTROLPACKETTF_H_
#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>
#include <anet/controlpacket.h>

namespace anet {
class ControlPacketTF : public CppUnit::TestFixture 
{
    CPPUNIT_TEST_SUITE(ControlPacketTF);
    CPPUNIT_TEST(testWhat);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp(){}
    void testWhat();
    void tearDown(){}
};
}

#endif /*CONTROLPACKETTF_H_*/
