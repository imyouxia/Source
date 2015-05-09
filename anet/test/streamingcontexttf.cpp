/**
 * $Id: streamingcontexttf.cpp 15518 2008-12-21 09:22:18Z zhangli $
 */

#include "streamingcontexttf.h"
#include "testadapter.h"
using namespace std;

namespace anet {
CPPUNIT_TEST_SUITE_REGISTRATION(StreamingContextTF);

void StreamingContextTF::setUp() {
    ConnPacket::_destructNum = 0;
    _context = new StreamingContext;
    CPPUNIT_ASSERT(_context);
}

void StreamingContextTF::tearDown() {
    if ( NULL != _context ) delete _context;
}

void StreamingContextTF::testIsAndSetCompleted() {
    CPPUNIT_ASSERT(!_context->isCompleted());
    _context->setCompleted(true);
    CPPUNIT_ASSERT(_context->isCompleted());
    _context->setCompleted(false);
    CPPUNIT_ASSERT(!_context->isCompleted());
}

void StreamingContextTF::testIsAndSetBroken() {
    CPPUNIT_ASSERT(!_context->isBroken());
    _context->setBroken(true);
    CPPUNIT_ASSERT(_context->isBroken());
    _context->setBroken(false);
    CPPUNIT_ASSERT(!_context->isBroken());
}

void StreamingContextTF::testIsAndSetEndOfFile() {
    CPPUNIT_ASSERT(!_context->isEndOfFile());
    _context->setEndOfFile(true);
    CPPUNIT_ASSERT(_context->isEndOfFile());
    _context->setEndOfFile(false);
    CPPUNIT_ASSERT(!_context->isEndOfFile());
}


void StreamingContextTF::testGetAndSetPacket() {
    CPPUNIT_ASSERT(!_context->getPacket());
    ConnPacket *packet = new ConnPacket;
    _context->setPacket(packet);
    CPPUNIT_ASSERT_EQUAL(packet, (ConnPacket*)_context->getPacket());
    packet = new ConnPacket;
    _context->setPacket(packet);
    CPPUNIT_ASSERT_EQUAL(1, ConnPacket::_destructNum);
    CPPUNIT_ASSERT_EQUAL(packet, (ConnPacket*)_context->getPacket());
    _context->setPacket(packet);
    CPPUNIT_ASSERT_EQUAL(1, ConnPacket::_destructNum);
    _context->setPacket(NULL);
    CPPUNIT_ASSERT_EQUAL(2, ConnPacket::_destructNum);
    CPPUNIT_ASSERT(!_context->getPacket());

}

void StreamingContextTF::testStealPacket() {
    CPPUNIT_ASSERT(!_context->stealPacket());
    ConnPacket *packet = new ConnPacket;
    ConnPacket *packet1 = new ConnPacket;
    _context->setPacket(packet);
    CPPUNIT_ASSERT_EQUAL(packet, (ConnPacket*)_context->stealPacket());
    CPPUNIT_ASSERT(!_context->getPacket());
    _context->setPacket(packet1);
    CPPUNIT_ASSERT_EQUAL(0, ConnPacket::_destructNum);
    CPPUNIT_ASSERT_EQUAL(packet1, (ConnPacket*)_context->stealPacket());
    CPPUNIT_ASSERT(!_context->getPacket());
    _context->setPacket(NULL);
    CPPUNIT_ASSERT(!_context->stealPacket());
    delete packet;
    delete packet1;
}

void StreamingContextTF::reset() {
    _context->reset();
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isEndOfFile());
    CPPUNIT_ASSERT(!_context->getPacket());
    CPPUNIT_ASSERT(!_context->getErrorNo());
    CPPUNIT_ASSERT(!_context->getErrorString());
    ConnPacket *packet = new ConnPacket;
    _context->setCompleted(true);
    _context->setBroken(true);
    _context->setEndOfFile(true);
    _context->setPacket(packet);
    _context->setErrorNo(3);
    _context->setErrorString("ERROR!");
    _context->reset();
    CPPUNIT_ASSERT_EQUAL(1, ConnPacket::_destructNum);
    CPPUNIT_ASSERT(!_context->isCompleted());
    CPPUNIT_ASSERT(!_context->isBroken());
    CPPUNIT_ASSERT(!_context->isEndOfFile());
    CPPUNIT_ASSERT(!_context->getPacket());
    CPPUNIT_ASSERT(!_context->getErrorNo());
    CPPUNIT_ASSERT(!_context->getErrorString());

}

void StreamingContextTF::testDestructor() {
    StreamingContext *context = new StreamingContext;
    ConnPacket *packet = new ConnPacket;
    context->setPacket(packet);
    delete context;
    CPPUNIT_ASSERT_EQUAL(1, ConnPacket::_destructNum);
}

void StreamingContextTF::testSetGetErrorNo() {
    StreamingContext *context = new StreamingContext;
    CPPUNIT_ASSERT_EQUAL(0, context->getErrorNo());
    context->setErrorNo(414);
    CPPUNIT_ASSERT_EQUAL(414, context->getErrorNo());
    context->setErrorNo(10);
    CPPUNIT_ASSERT_EQUAL(10, context->getErrorNo());
    context->setErrorNo(-400);
    CPPUNIT_ASSERT_EQUAL(-400, context->getErrorNo());
    delete context;

}

void StreamingContextTF::testSetGetErrorString() {
    StreamingContext *context = new StreamingContext;
    CPPUNIT_ASSERT_EQUAL((const char*)NULL, context->getErrorString());
    context->setErrorString("");
    CPPUNIT_ASSERT_EQUAL(string(""), string(context->getErrorString()));
    context->setErrorString("abc");
    CPPUNIT_ASSERT_EQUAL(string("abc"), string(context->getErrorString()));
    delete context;
}

}/*end namespace anet*/
