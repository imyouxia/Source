/**
 * File name: defaultpackettf.h
 * Author: zhangli
 * Create time: 2008-12-25 11:10:50
 * $Id$
 * 
 * Description: ***add description here***
 * 
 */

#ifndef ANET_DEFAULTPACKETTF_H_
#define ANET_DEFAULTPACKETTF_H_
#include <cppunit/extensions/HelperMacros.h>
#include <anet/defaultpacket.h>
#include <anet/databuffer.h>

namespace anet {
class DefaultPacketTF : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(DefaultPacketTF);
    CPPUNIT_TEST(testSetGetBody);
    CPPUNIT_TEST(testEncodeDecode);
    CPPUNIT_TEST(testSetGetCapacity);
    CPPUNIT_TEST_SUITE_END();
public:
    void setUp();
    void tearDown();
    void testSetGetBody();
    void testEncodeDecode();
    void testSetGetCapacity();
private:
    DefaultPacket *_packet;
    DataBuffer *_dataBuffer;
};

}/*end namespace anet*/
#endif/* ANET_DEFAULTPACKETTF_H_*/
