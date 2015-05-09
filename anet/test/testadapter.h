#ifndef	TESTADAPTER_H_
#define TESTADAPTER_H_
#define CONN_MAX_SIZE 4096
#include <anet/anet.h>
#include <anet/defaultpacket.h>
#include <anet/log.h>

namespace anet{
class ConnectionForTest : public Connection{
public:
    ConnectionForTest(Socket *socket, IPacketStreamer *streamer, IServerAdapter *serverAdapter)
        : Connection(socket, streamer, serverAdapter){}

    ~ConnectionForTest(){}

    bool writeData(){
        return false;
    }
		
    bool readData(){
        return false;
    }
};
    
class ConnPacket : public DefaultPacket{
public:
    ConnPacket(int dataLen = 16, int actualLen = -1) {
        _packetHeader._dataLen = dataLen;
        if(actualLen == -1) _actualLen = dataLen;
        else _actualLen = actualLen;
        _str[0] = '\0';
    }
    
    ~ConnPacket(){
        ANET_LOG(SPAM,"delete ConnPacket(%p)", this);
        ConnPacket:: _destructNum ++;
    }
    
    void setString(const char *str) {
        strncpy(_str, str, CONN_MAX_SIZE);
        _str[CONN_MAX_SIZE-1] = '\0';
    }
    
    char *getString() {
        return _str;
    }
    bool encode(DataBuffer *output) {
        output->writeBytes("data for test", _actualLen);
        return true;
    }

    bool decode(DataBuffer *input, PacketHeader *header)
    {
        int len = header->_dataLen;
        if (len >= CONN_MAX_SIZE) {
            len = CONN_MAX_SIZE - 1;
        }
        input->readBytes(_str, len);
        _str[len] = '\0';
        if (header->_dataLen > len) {
            input->drainData(header->_dataLen - len);
        }
        return true;

    }
 
private:
    char _str[CONN_MAX_SIZE];
    int _actualLen;
public:
    static int _destructNum;
};


class ConnPacketFactory : public IPacketFactory{
public:
    Packet *createPacket(int pcode){
        return new ConnPacket();
    }
};

class DefaultPacketHandler : public IPacketHandler
{
public:
    HPRetCode handlePacket(Packet *packet, void *args)
    {
        packet->free();
        return CLOSE_CHANNEL;
    }
};
class ConnServerAdapter : public IServerAdapter
{
public:
    IPacketHandler::HPRetCode handlePacket(Connection *connection, Packet *packet)
    {
	ConnPacket *reply = new ConnPacket();
        reply->setString(((ConnPacket*)packet)->getString());
        reply->setChannelId(packet->getChannelId());
        connection->postPacket(reply);
        packet->free();
        return IPacketHandler::FREE_CHANNEL;
    }    
};    
class ConnComponent : public IOComponent{
public:
    ConnComponent(Socket *socket):IOComponent((Transport*)0xabcdef, socket){
        ;
    }
    bool handleWriteEvent(){
        return true;
    }
    bool handleReadEvent(){
        return true;
    }
    bool handleErrorEvent(){
        return true;
    }
    bool init(bool isServer = false){
        return true;
    }
    bool checkTimeout(int64_t now){
        return true;
    }
};
}

#endif /* TESTADAPTER_H_*/
