/**
 * $Id: transporttf.h 15749 2008-12-30 02:49:58Z zhangli $
 */
   
#ifndef TRANSPORTTF_H_
#define TRANSPORTTF_H_
#include <cppunit/extensions/HelperMacros.h>
#include <anet/anet.h>
#include <anet/log.h>

#define DATA_MAX_SIZE 4096

namespace anet {
class EchoServerAdapter;
class EchoPacketHandler;  
class TransportTF : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(TransportTF);
    CPPUNIT_TEST(testListen);
    CPPUNIT_TEST(testConnect);
    CPPUNIT_TEST(testAddComponent);
    CPPUNIT_TEST(testEventIteration);
    CPPUNIT_TEST(testTimeoutIteration);
    CPPUNIT_TEST(testClosedByUser);
    CPPUNIT_TEST(testAutoReconnect);
    CPPUNIT_TEST(testConstructor);
    CPPUNIT_TEST(testRun);
    CPPUNIT_TEST_SUITE_END();
public:
    TransportTF();
    ~TransportTF();
    void testListen();
    void testConnect();
    void testAddComponent();
    void testEventIteration();
    void testTimeoutIteration();
    void testClosedByUser();
    void testAutoReconnect();
    void testConstructor();
    void testRun();
private:
    IPacketFactory *_factory;
    IPacketStreamer *_streamer;
    EchoServerAdapter *_adapter;
    EchoPacketHandler *_handler;
};

class EchoPacket : public Packet {
public:
    EchoPacket() {
        _str[0] = '\0';
    }
    
    void setString(char *str) {
        strncpy(_str, str, DATA_MAX_SIZE);
        _str[DATA_MAX_SIZE-1] = '\0';
    }
    
    char *getString() {
        return _str;
    }

    bool encode(DataBuffer *output) {
        output->writeBytes(_str, strlen(_str));
        return true;
    }

    bool decode(DataBuffer *input, PacketHeader *header) {
        int len = header->_dataLen;
        if (len >= DATA_MAX_SIZE) {
            len = DATA_MAX_SIZE - 1;
        }
        input->readBytes(_str, len);
        _str[len] = '\0';
        if (header->_dataLen > len) {
            input->drainData(header->_dataLen - len);
        }
        return true;
    }
    
    void free()
    {
        delete this;
    }
    
public:
    char _str[DATA_MAX_SIZE];
};

class EchoPacketFactory : public IPacketFactory
{
public:
    Packet *createPacket(int pcode)
    {
        return new EchoPacket();
    }
};

class EchoPacketHandler : public IPacketHandler {
public:
    EchoPacketHandler() {
        atomic_set(&_count, 0);
        atomic_set(&_errorcount, 0);
        atomic_set(&_timeoutCount, 0);
        atomic_set(&_closeCount, 0);
        atomic_set(&_badCount, 0);
    }
    HPRetCode handlePacket(Packet *packet, void *args)
    {
        atomic_inc(&_count);
        if (!packet->isRegularPacket()) { // 是否正常的包
            atomic_inc(&_errorcount);
            ANET_LOG(ERROR, "=> ControlPacket: %d, index:%d", 
                     ((ControlPacket*)packet)->getCommand(), (long)args);
            switch(((ControlPacket*)packet)->getCommand()) {
            case ControlPacket::CMD_BAD_PACKET:
                atomic_inc(&_badCount);
                break;
            case ControlPacket::CMD_TIMEOUT_PACKET:
                atomic_inc(&_timeoutCount);
                break;
            case ControlPacket::CMD_CONNECTION_CLOSED:
                atomic_inc(&_closeCount);
                break;
            default:
                break;
            }
        } else {
            packet->free();
        }
        return IPacketHandler::FREE_CHANNEL;
    }    
    atomic_t _count;
    atomic_t _errorcount;
    atomic_t _timeoutCount;
    atomic_t _closeCount;
    atomic_t _badCount;
};

class EchoServerAdapter : public IServerAdapter, public EchoPacketHandler{
public:
    IPacketHandler::HPRetCode handlePacket(Connection *connection,
            Packet *packet) {
        EchoPacket *reply = new EchoPacket();
        reply->setString(((EchoPacket*)packet)->getString());
        reply->setChannelId(packet->getChannelId());
        EchoPacketHandler::handlePacket(packet,NULL);
        connection->postPacket(reply);
        return IPacketHandler::FREE_CHANNEL;
    }
};
}

#endif /*TRANSPORTTF_H_*/

