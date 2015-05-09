#include <anet/anet.h>
#include "httppacketstreamertf.h"
#include <anet/httppacketstreamer.h>


using namespace std;

namespace anet{
CPPUNIT_TEST_SUITE_REGISTRATION(HttpPacketStreamerTF);

void HttpPacketStreamerTF::setUp() {
}

void HttpPacketStreamerTF::tearDown() {
}
    

void HttpPacketStreamerTF::testGetPacketInfo() {
    DataBuffer input;
    PacketHeader header;
    bool broken;
    DefaultHttpPacketFactory *factory =  new DefaultHttpPacketFactory();
    HttpPacketStreamer streamer(factory);
    input.writeBytes("wrong", 5);
    CPPUNIT_ASSERT(!streamer.getPacketInfo(&input, &header, &broken));
    input.drainData(6);
    input.writeBytes("GET ", 4);
    CPPUNIT_ASSERT(!streamer.getPacketInfo(&input, &header, &broken));
    input.writeBytes("this is the content ", 16);
    input.writeBytes("\r\n\r\n", 4);    
    CPPUNIT_ASSERT(streamer.getPacketInfo(&input, &header, &broken));
    CPPUNIT_ASSERT_EQUAL(24, header._dataLen);
    delete factory;
}
}
