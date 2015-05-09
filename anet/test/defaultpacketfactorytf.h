/**
 * File name: defaultpacketfactorytf.h
 * Author: zhangli
 * Create time: 2008-12-25 11:29:15
 * $Id$
 * 
 * Description: ***add description here***
 * 
 */

#ifndef ANET_DEFAULTPACKETFACTORYTF_H_
#define ANET_DEFAULTPACKETFACTORYTF_H_
#include <cppunit/extensions/HelperMacros.h>

namespace anet {
class DefaultPacketFactoryTF : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(DefaultPacketFactoryTF);
    CPPUNIT_TEST(testDefaultPacketFactory);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();
    void testDefaultPacketFactory();
};

}/*end namespace anet*/
#endif/* ANET_DEFAULTPACKETFACTORYTF_H_*/
