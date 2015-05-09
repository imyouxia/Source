#include <anet/log.h>
#include "defaultpacketstreamertf.h"
#include <anet/streamingcontext.h>
#include <anet/defaultpacketstreamer.h>
#include <anet/defaultpacketfactory.h>
#include <anet/defaultpacket.h>
#include <anet/httppacketstreamer.h>
using namespace std;

namespace anet{
CPPUNIT_TEST_SUITE_REGISTRATION(DEFAULTPACKETSTREAMERTF);

class MyPacket : public Packet
{
public:
       
    MyPacket(int dataLen, bool goodPacket = true)
        : _dataLen(dataLen), _goodPacket(goodPacket) {}
    
    bool encode(DataBuffer *output) {
        output->writeBytes("data for test", _dataLen);
        if (_goodPacket) return true;
        else return false;
    }

        
    bool decode(DataBuffer *input, PacketHeader *header)
    {
        input->drainData(1);
        return true;
    }
    
private:
    int _dataLen;
    bool _goodPacket;
};

class MyPacketFactory : public IPacketFactory
{
public:
    MyPacketFactory(bool identify = true):_identify(identify) {
    }
        
    Packet *createPacket(int pcode)
    {

        if (_identify)
        {
            return new MyPacket(0);
        }
        else return NULL;
        
    }
private:
    bool _identify;
};

void DEFAULTPACKETSTREAMERTF::setUp() {
    ;
} 
void DEFAULTPACKETSTREAMERTF::tearDown() {
    ;
}

void DEFAULTPACKETSTREAMERTF::testGetPacketInfo() {
    DataBuffer buffer;
    PacketHeader header;
    DefaultPacketStreamer streamer(NULL);
    bool broken;
    buffer.writeInt32(ANET_PACKET_FLAG);
    buffer.writeInt32(1111);
    buffer.writeInt32(2222);
    buffer.writeInt32(-1);    
    buffer.writeBytes("data for test", 16);
    CPPUNIT_ASSERT(!streamer.getPacketInfo(&buffer, &header, &broken));
    CPPUNIT_ASSERT_EQUAL(1111, (int)header._chid);
    CPPUNIT_ASSERT_EQUAL(2222, (int)header._pcode);
    CPPUNIT_ASSERT_EQUAL(-1, (int)header._dataLen);
    

    CPPUNIT_ASSERT(broken);

    buffer.drainData(buffer.getDataLen());
    streamer._existPacketHeader = false;
    CPPUNIT_ASSERT(!streamer.getPacketInfo(&buffer, &header, &broken));

    
}

void DEFAULTPACKETSTREAMERTF::testDecode() {
//     MyPacketFactory *factory =  new MyPacketFactory(true);
//     DefaultPacketStreamer *streamer
//         = new DefaultPacketStreamer(factory);
//     PacketHeader header;
//     header._pcode = ANET_PACKET_FLAG ;
//     header._dataLen = 16;
//     DataBuffer buffer;
//     buffer.writeBytes("data for test", 17);
   
//     Packet *packet = streamer->decode(&buffer, &header);
//     CPPUNIT_ASSERT(packet);
//     CPPUNIT_ASSERT_EQUAL(16, buffer.getDataLen());
//     delete streamer;
//     delete packet;
//     delete factory;
//     factory =  new MyPacketFactory(false);
//     streamer = new DefaultPacketStreamer(factory);
//     CPPUNIT_ASSERT(!streamer->decode(&buffer, &header));
//     CPPUNIT_ASSERT_EQUAL(0, buffer.getDataLen());
//     delete streamer;
//     delete factory;
}

void DEFAULTPACKETSTREAMERTF::testEncode() {
    MyPacketFactory *factory =  new MyPacketFactory();
    DefaultPacketStreamer *streamer
        = new DefaultPacketStreamer(factory);
    MyPacket packet(16);
    DataBuffer buffer;
    PacketHeader *header = new PacketHeader;
    packet.setPacketHeader(header);
    delete header;
    header = packet.getPacketHeader();
    header->_chid = 1111;
    header->_pcode = 2222;
    CPPUNIT_ASSERT(streamer->encode(&packet, &buffer));
    CPPUNIT_ASSERT_EQUAL(32, (int)buffer.getDataLen());
    buffer.drainData(4);
    CPPUNIT_ASSERT_EQUAL(1111, (int)buffer.readInt32());
    CPPUNIT_ASSERT_EQUAL(2222, (int)buffer.readInt32());
    CPPUNIT_ASSERT_EQUAL(16, (int)buffer.readInt32());


    MyPacket packet1(100, false);
    CPPUNIT_ASSERT(!streamer->encode(&packet1, &buffer));
    CPPUNIT_ASSERT_EQUAL(16, (int)buffer.getDataLen());
    delete streamer;
    delete factory;
}

void DEFAULTPACKETSTREAMERTF::testProcessData() {
    DefaultPacketFactory *factory =  new DefaultPacketFactory();
    DefaultPacketStreamer *streamer = new DefaultPacketStreamer(factory);
    DataBuffer *buffer = new DataBuffer;
    StreamingContext *context = streamer->createContext();
    CPPUNIT_ASSERT(context);

    DefaultPacket *packet = new DefaultPacket;
    DefaultPacket *result = NULL;
    size_t bodyLength;
    CPPUNIT_ASSERT(streamer->processData(buffer, context));
    CPPUNIT_ASSERT(!context->isBroken());
    CPPUNIT_ASSERT(!context->isCompleted());

    packet->setBody("",0);
    CPPUNIT_ASSERT(streamer->encode(packet, buffer));
    CPPUNIT_ASSERT(streamer->processData(buffer, context));
    CPPUNIT_ASSERT(!context->isBroken());
    CPPUNIT_ASSERT(context->isCompleted());
    result = dynamic_cast<DefaultPacket*>(context->getPacket());
    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT(!result->getBody(bodyLength));
    CPPUNIT_ASSERT_EQUAL((size_t)0, bodyLength);
    context->reset();

    packet->setBody("a",1);
    CPPUNIT_ASSERT(streamer->encode(packet, buffer));
    CPPUNIT_ASSERT(streamer->processData(buffer, context));
    CPPUNIT_ASSERT(!context->isBroken());
    CPPUNIT_ASSERT(context->isCompleted());
    result = dynamic_cast<DefaultPacket*>(context->getPacket());
    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT(result->getBody(bodyLength));
    CPPUNIT_ASSERT_EQUAL('a', *(result->getBody(bodyLength)));
    CPPUNIT_ASSERT_EQUAL((size_t)1, bodyLength);
    context->reset();

    packet->setBody("ab",2);
    CPPUNIT_ASSERT(streamer->encode(packet, buffer));
    packet->setBody("a",1);
    CPPUNIT_ASSERT(streamer->encode(packet, buffer));
    CPPUNIT_ASSERT(streamer->processData(buffer, context));
    CPPUNIT_ASSERT(!context->isBroken());
    CPPUNIT_ASSERT(context->isCompleted());
    result = dynamic_cast<DefaultPacket*>(context->getPacket());
    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT(result->getBody(bodyLength));
    CPPUNIT_ASSERT_EQUAL('a', *(result->getBody(bodyLength)));
    CPPUNIT_ASSERT_EQUAL('b', *(result->getBody(bodyLength) + 1));
    CPPUNIT_ASSERT_EQUAL((size_t)2, bodyLength);
    context->reset();
    CPPUNIT_ASSERT(streamer->processData(buffer, context));
    CPPUNIT_ASSERT(!context->isBroken());
    CPPUNIT_ASSERT(context->isCompleted());
    result = dynamic_cast<DefaultPacket*>(context->getPacket());
    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT(result->getBody(bodyLength));
    CPPUNIT_ASSERT_EQUAL('a', *(result->getBody(bodyLength)));
    CPPUNIT_ASSERT_EQUAL((size_t)1, bodyLength);
    context->reset();


    delete streamer;
    delete factory;
    delete buffer;
    delete packet;
    delete context;
}

void DEFAULTPACKETSTREAMERTF::testProcessDataMultiStep() {
    DefaultPacketFactory *factory =  new DefaultPacketFactory();
    DefaultPacketStreamer *streamer = new DefaultPacketStreamer(factory);
    DataBuffer *buffer = new DataBuffer;
    StreamingContext *context = streamer->createContext();
    CPPUNIT_ASSERT(context);

    DefaultPacket *packet = new DefaultPacket;
    DefaultPacket *result = NULL;
    size_t bodyLength;
    /*need to multi step here*/

    buffer->writeInt32(ANET_PACKET_FLAG);
    CPPUNIT_ASSERT(streamer->processData(buffer, context));
    CPPUNIT_ASSERT(!context->isBroken());
    CPPUNIT_ASSERT(!context->isCompleted());

    buffer->writeInt32(0xabcd);
    CPPUNIT_ASSERT(streamer->processData(buffer, context));
    CPPUNIT_ASSERT(!context->isBroken());
    CPPUNIT_ASSERT(!context->isCompleted());

    buffer->writeInt32(1);
    CPPUNIT_ASSERT(streamer->processData(buffer, context));
    CPPUNIT_ASSERT(!context->isBroken());
    CPPUNIT_ASSERT(!context->isCompleted());

    buffer->writeInt32(10);
    CPPUNIT_ASSERT(streamer->processData(buffer, context));
    CPPUNIT_ASSERT(!context->isBroken());
    CPPUNIT_ASSERT(!context->isCompleted());

    CPPUNIT_ASSERT(streamer->processData(buffer, context));
    CPPUNIT_ASSERT(!context->isBroken());
    CPPUNIT_ASSERT(!context->isCompleted());

    buffer->writeBytes("1", 1);
    CPPUNIT_ASSERT(streamer->processData(buffer, context));
    CPPUNIT_ASSERT(!context->isBroken());
    CPPUNIT_ASSERT(!context->isCompleted());

    buffer->writeBytes("2", 1);
    CPPUNIT_ASSERT(streamer->processData(buffer, context));
    CPPUNIT_ASSERT(!context->isBroken());
    CPPUNIT_ASSERT(!context->isCompleted());

    buffer->writeBytes("34567890", 8);
    CPPUNIT_ASSERT(streamer->processData(buffer, context));
    CPPUNIT_ASSERT(!context->isBroken());
    CPPUNIT_ASSERT(context->isCompleted());

    result = dynamic_cast<DefaultPacket*>(context->getPacket());
    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT(result->getBody(bodyLength));
    CPPUNIT_ASSERT_EQUAL((size_t)10, bodyLength);
    CPPUNIT_ASSERT_EQUAL(0xabcdu, result->getChannelId());
    CPPUNIT_ASSERT_EQUAL(1, result->getPacketHeader()->_pcode);
    context->reset();
    delete streamer;
    delete factory;
    delete buffer;
    delete packet;
    delete context;
}

void DEFAULTPACKETSTREAMERTF::testProcessDataErrorHttpRequestPacket() {
    ANET_LOG(DEBUG,"Begin testProcessDataErrorHttpRequestPacket");
    DefaultHttpPacketFactory *factory =  new DefaultHttpPacketFactory();
    HttpPacketStreamer *streamer = new HttpPacketStreamer(factory);
    DataBuffer *buffer = new DataBuffer;
    StreamingContext *context = streamer->createContext();
    CPPUNIT_ASSERT(context);
    const char* errorHeader = "GET atomic_read(&globalInfo->"
      "_failedQuery) - HTTP/1.1\r\n"
      "Host:localhost\r\n"
      "Connection:close\r\n"
      "User-Agent:abench 1.6\r\n\r\n";

    buffer->writeBytes(errorHeader, strlen(errorHeader));
    Packet *request = new HttpRequestPacket();
    PacketHeader header;
    bool broken = false;
    CPPUNIT_ASSERT(streamer->getPacketInfo(buffer, &header, &broken));
    CPPUNIT_ASSERT_EQUAL(strlen(errorHeader), (size_t)header._dataLen);
    CPPUNIT_ASSERT(!broken);
    CPPUNIT_ASSERT(!request->decode(buffer, &header));
    request->free();

    ANET_LOG(DEBUG,"Before test packet created from factory ...");
    buffer->writeBytes(errorHeader, strlen(errorHeader));
    CPPUNIT_ASSERT(streamer->getPacketInfo(buffer, &header, &broken));
    CPPUNIT_ASSERT_EQUAL(strlen(errorHeader), (size_t)header._dataLen);
    CPPUNIT_ASSERT(!broken);
    request = factory->createPacket(header._pcode);
    CPPUNIT_ASSERT(!request->decode(buffer, &header));
    request->free();

    ANET_LOG(DEBUG,"Before test processData ...");
    CPPUNIT_ASSERT(!context->getPacket());
    buffer->writeBytes(errorHeader, strlen(errorHeader));
    CPPUNIT_ASSERT(streamer->processData(buffer, context));
    CPPUNIT_ASSERT(context->isCompleted());
    //    CPPUNIT_ASSERT(context->isBroken());
    context->reset();
    delete streamer;
    delete factory;
    delete buffer;
    delete context;
    ANET_LOG(DEBUG,"End testProcessDataErrorHttpRequestPacket");
}

}
