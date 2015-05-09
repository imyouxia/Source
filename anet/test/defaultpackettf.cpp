/**
 * File name: defaultpackettf.cpp
 * Author: zhangli
 * Create time: 2008-12-25 11:10:50
 * $Id$
 * 
 * Description: ***add description here***
 * 
 */

#include "defaultpackettf.h"
#include <anet/defaultpacket.h>
#include <anet/log.h>
using namespace std;

namespace anet {
CPPUNIT_TEST_SUITE_REGISTRATION(DefaultPacketTF);

void DefaultPacketTF::setUp() {
    _packet = new DefaultPacket;
    _dataBuffer = new DataBuffer;
}

void DefaultPacketTF::tearDown() {
    delete _packet;
    delete _dataBuffer;
}

void DefaultPacketTF::testSetGetBody() {
    string body = "http://localhost:9090/1.php?a\x1fvery looooooooooooooong!";
    string spaces = "  ";
    char* const null = NULL;
    char five[5] = { '3', '\0', 'a', '\t', ' ' };
    size_t len;
    const char *result = NULL;

    CPPUNIT_ASSERT(!_packet->getBody(len));
    CPPUNIT_ASSERT_EQUAL((size_t)0, len);

    CPPUNIT_ASSERT(_packet->setBody(spaces.c_str(), spaces.length()));
    CPPUNIT_ASSERT(_packet->getBody(len));
    CPPUNIT_ASSERT_EQUAL(spaces.length(), len);
    
    CPPUNIT_ASSERT(!_packet->setBody(null,3));
    CPPUNIT_ASSERT(_packet->getBody(len));
    CPPUNIT_ASSERT_EQUAL(spaces.length(), len);

    CPPUNIT_ASSERT(_packet->setBody(null,0));
    CPPUNIT_ASSERT(!_packet->getBody(len));
    CPPUNIT_ASSERT_EQUAL((size_t)0, len);

    CPPUNIT_ASSERT(_packet->setBody(five, 5));
    result = _packet->getBody(len);
    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT_EQUAL((size_t)5, len);
    CPPUNIT_ASSERT_EQUAL(0, memcmp(result, five, 5));

    CPPUNIT_ASSERT(_packet->setBody(body.c_str(),0));
    CPPUNIT_ASSERT(!_packet->getBody(len));
    CPPUNIT_ASSERT_EQUAL((size_t)0, len);

    CPPUNIT_ASSERT(_packet->appendBody(five, 5));
    result = _packet->getBody(len);
    CPPUNIT_ASSERT_EQUAL((size_t)5, len);
    CPPUNIT_ASSERT_EQUAL(0, memcmp(result, five, 5));

    CPPUNIT_ASSERT(!_packet->appendBody(null, 4));
    result = _packet->getBody(len);
    CPPUNIT_ASSERT_EQUAL((size_t)5, len);
    CPPUNIT_ASSERT_EQUAL(0, memcmp(result, five, 5));

    CPPUNIT_ASSERT(_packet->appendBody(body.c_str(), body.length()));
    result = _packet->getBody(len);
    CPPUNIT_ASSERT_EQUAL(body.length() + 5, len);
    CPPUNIT_ASSERT_EQUAL(0, memcmp(result, five, 5));
    CPPUNIT_ASSERT_EQUAL(0, memcmp(result + 5, body.c_str(), body.length()));

    CPPUNIT_ASSERT(!_packet->appendBody(five, 0));
    result = _packet->getBody(len);
    CPPUNIT_ASSERT_EQUAL(body.length() + 5, len);
    CPPUNIT_ASSERT_EQUAL(0, memcmp(result, five, 5));
    CPPUNIT_ASSERT_EQUAL(0, memcmp(result + 5, body.c_str(), body.length()));
}

void DefaultPacketTF::testEncodeDecode() {
    PacketHeader *header = _packet->getPacketHeader();
    size_t bodyLength = 0;
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT_EQUAL(0, _dataBuffer->getDataLen());
    header->_dataLen = 0;
    CPPUNIT_ASSERT(_packet->decode(_dataBuffer, header));
    CPPUNIT_ASSERT(!_packet->getBody(bodyLength));

    _packet->setBody("", 0);
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT_EQUAL(0, _dataBuffer->getDataLen());
    header->_dataLen = 0;
    CPPUNIT_ASSERT(_packet->decode(_dataBuffer, header));
    CPPUNIT_ASSERT(!_packet->getBody(bodyLength));
    
    _packet->setBody("1", 1);
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT_EQUAL(1, _dataBuffer->getDataLen());
    CPPUNIT_ASSERT_EQUAL('1', *(_dataBuffer->getData()));
    header->_dataLen = 1;
    CPPUNIT_ASSERT(_packet->decode(_dataBuffer, header));
    CPPUNIT_ASSERT_EQUAL('1', *(_packet->getBody(bodyLength)));
    CPPUNIT_ASSERT_EQUAL((size_t)1, bodyLength);

    ANET_LOG(DEBUG, "before decode bug, buffer length(%d)",
             _dataBuffer->getDataLen());
    _packet->setBody("\x00\x1f", 2);
    CPPUNIT_ASSERT(_packet->encode(_dataBuffer));
    CPPUNIT_ASSERT_EQUAL(2, _dataBuffer->getDataLen());
    CPPUNIT_ASSERT_EQUAL('\x00', *(_dataBuffer->getData()));
    CPPUNIT_ASSERT_EQUAL('\x1f', *(_dataBuffer->getData() + 1));
    header->_dataLen = 2;
    CPPUNIT_ASSERT(_packet->decode(_dataBuffer, header));
    CPPUNIT_ASSERT_EQUAL('\x00',*(_packet->getBody(bodyLength)));
    CPPUNIT_ASSERT_EQUAL('\x1f',*(_packet->getBody(bodyLength) + 1));
    CPPUNIT_ASSERT_EQUAL((size_t)2, bodyLength);
}

void DefaultPacketTF::testSetGetCapacity() {
    ANET_LOG(DEBUG, "Begin testSetGetCapacity()");
    DefaultPacket packet;
    CPPUNIT_ASSERT(packet.setCapacity(0));
    CPPUNIT_ASSERT_EQUAL((size_t)0, packet.getBodyLen());
    CPPUNIT_ASSERT_EQUAL((size_t)0, packet.getCapacity());
    CPPUNIT_ASSERT(packet.appendBody("a",1));
    CPPUNIT_ASSERT_EQUAL((size_t)1, packet.getBodyLen());
    CPPUNIT_ASSERT(packet.getCapacity() > packet.getBodyLen());
    CPPUNIT_ASSERT(!packet.setCapacity(0));
    CPPUNIT_ASSERT(packet.setCapacity(1));
    CPPUNIT_ASSERT_EQUAL((size_t)1, packet.getCapacity());
    CPPUNIT_ASSERT_EQUAL((size_t)1, packet.getBodyLen());
    CPPUNIT_ASSERT(packet.setCapacity(1024));
    CPPUNIT_ASSERT_EQUAL((size_t)1, packet.getBodyLen());
    CPPUNIT_ASSERT_EQUAL((size_t)1024, packet.getCapacity());
    CPPUNIT_ASSERT(packet.setBody("1234567890",10));
    CPPUNIT_ASSERT_EQUAL((size_t)10, packet.getBodyLen());
    CPPUNIT_ASSERT_EQUAL((size_t)1024, packet.getCapacity());
    CPPUNIT_ASSERT(!packet.setCapacity(9));
    CPPUNIT_ASSERT(packet.setCapacity(11));
    CPPUNIT_ASSERT(packet.setCapacity(10));
    ANET_LOG(DEBUG, "End testSetGetCapacity()");
}

}/*end namespace anet*/
