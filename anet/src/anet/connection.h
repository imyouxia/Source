#ifndef ANET_CONNECTION_H_
#define ANET_CONNECTION_H_

#define READ_WRITE_SIZE 8192
#include <anet/packetqueue.h>
#include <anet/threadcond.h>
#include <anet/channelpool.h>
#include <anet/ipacketstreamer.h>
namespace anet {
class IPacketHandler;
class Packet;
class IOComponent;
class IServerStreamer;
class IServerAdapter;
class Socket;
class Connection {
    friend class ConnectionTF;
    friend class TCPCONNECTIONTF;
public:

    Connection(Socket *socket, IPacketStreamer *streamer, IServerAdapter *serverAdapter);

    virtual ~Connection();

    /**
     * 设置是否为服务器端
     */
    void setServer(bool isServer) {
        _isServer = isServer;
    }

    bool isServer() {
        return _isServer;
    }

    void setIOComponent(IOComponent *ioc) {
        _iocomponent = ioc;
    }

    IOComponent *getIOComponent() {
        return _iocomponent;
    }

    /**
     * 设置默认的packetHandler
     */
    void setDefaultPacketHandler(IPacketHandler *ph) {
        _defaultPacketHandler = ph;
    }

    /**
     * 发送packet到发送队列
     *
     * @param packet 数据包
     * @param packetHandler 
     * @param args 自定义参数
     * @param blocking whether blocking if output queue is full.
     */
    bool postPacket(Packet *packet, IPacketHandler *packetHandler = NULL,
		    void *args = NULL, bool blocking = false);

    /**
     * Send a request packet to server, then waiting for the replay packet.
     * If the return value is NULL, caller is responsible for freeing the
     * request packet; If the return value is NOT NULL, ANet will free the
     * request packet, and caller is responsible for freeing the reply packet.
     *
     * @param packet the request to be sent to server
     * @return the value returned is a pointer to a reply packet received
     * from server, or NULL if fail to send the reply.
     */
    Packet *sendPacket(Packet *packet);

    /**
     * 当数据收到时的处理函数
     */
    bool handlePacket(Packet *header);

    uint32_t getPacketPostedCount() {return _packetPostedCount;}
    uint32_t getPacketHandledCount() {return _packetHandledCount;}
    /**
     * 检查超时
     */
    bool checkTimeout(int64_t now);

    void closeHook();

    virtual bool writeData() = 0;

    virtual bool readData() = 0;

    /**
     * 设置写完是否关闭, 只TCP要用
     */
    virtual void setWriteFinishClose(bool v) {
        ;
    }

    /**
     * 设置对列的超时时间
     */
    void setQueueTimeout(int queueTimeout) {
        _queueTimeout = queueTimeout;
    }

    /**
     * 清空output的buffer
     */
    virtual void clearOutputBuffer() {
        ;
    }

    /**
     * 清空input的buffer
     */
    virtual void clearInputBuffer() {
        ;
    }
    
    
    /**
     * 设置queue最大长度, 0 - 不限制
     */
    bool setQueueLimit(size_t limit); 
    size_t getQueueLimit();

    virtual void close();
    virtual void addRef();
    virtual void subRef();
    virtual int getRef();

    virtual bool isClosed();

    void wakeUpSender();
    void beforeCallback();
    void afterCallback();

    void resetContext(); 
    StreamingContext* getContext() {
        return _context;
    }
protected:
    IPacketHandler *_defaultPacketHandler;  // connection的默认的packet handler
    bool _isServer;                         
    IOComponent *_iocomponent;
    Socket *_socket;                        
    IPacketStreamer *_streamer;             // Packet Streamer
    IServerAdapter *_serverAdapter;         // Server Adapter

    PacketQueue  _outputQueue;              // Output Queue
    PacketQueue _myQueue;	            // temporary queue used in write data
    ThreadCond _outputCond;                 // output condition
    ChannelPool _channelPool;
    int _queueTimeout;                      // tiemout spesification (millseconds)
    size_t _queueLimit;                     // max pending set packet allowed
    int _waitingThreads;
    bool _closed;
    StreamingContext *_context;
    uint32_t _packetPostedCount;
    uint32_t _packetHandledCount;
};
}

#endif /*CONNECTION_H_*/

