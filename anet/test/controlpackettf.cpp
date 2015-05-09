#include <iostream>
#include <string>
#include "controlpackettf.h"
#include <unistd.h>
using namespace std;
namespace anet {
CPPUNIT_TEST_SUITE_REGISTRATION(ControlPacketTF);

void ControlPacketTF::testWhat() {
    static const char *badStr = "Bad Packet received";
    static const char *timeoutStr = "Packet Timeout";
    static const char *closing = "Connection closing";
    const char *r = NULL;

    ControlPacket *badPacket = &ControlPacket::BadPacket;
    r = badPacket->what();
    CPPUNIT_ASSERT_EQUAL(string(badStr),string(r));

    ControlPacket *timeoutPacket = &ControlPacket::TimeoutPacket;
    r = timeoutPacket->what();
    CPPUNIT_ASSERT_EQUAL(string(timeoutStr),string(r));

    ControlPacket *closingPacket = &ControlPacket::ConnectionClosedPacket;
    r = closingPacket->what();
    CPPUNIT_ASSERT_EQUAL(string(closing),string(r));
}
}
