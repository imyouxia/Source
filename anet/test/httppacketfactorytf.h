/**
 * File name: httppacketfactorytf.h
 * Author: zhangli
 * Create time: 2008-12-19 16:36:39
 * $Id$
 * 
 * Description: ***add description here***
 * 
 */

#ifndef ANET_HTTPPACKETFACTORYTF_H_
#define ANET_HTTPPACKETFACTORYTF_H_
#include <cppunit/extensions/HelperMacros.h>

namespace anet {
class HTTPPacketFactoryTF : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(HTTPPacketFactoryTF);
    CPPUNIT_TEST(testHTTPPacketFactory);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();
    void testHTTPPacketFactory();
};

}/*end namespace anet*/
#endif/* ANET_HTTPPACKETFACTORYTF_H_*/
