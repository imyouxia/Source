#ifndef ANET_IOCOMPONENT_H_
#define ANET_IOCOMPONENT_H_

#include <anet/atomic.h>
#include <anet/threadmutex.h>
#include <anet/socket.h>

namespace anet {
const int MAX_RECONNECTING_TIMES = 30;
const int64_t MAX_IDLE_TIME = 900000000;//15 minutes
const int64_t CONNECTING_TIMEOUT = 2000000;//2 seconds
const int64_t RECONNECTING_INTERVAL = 1000000;//1 seconds

class Connection;
class SocketEvent;
class IPacketStreamer;
class IServerAdapter;
class Socket;
class Transport;
class IOComponent {
    friend class Transport;
    friend class TCPCOMPONENTTF;
    friend class ConnecionTF;
    friend class TCPCONNECTIONTF;
public:
    enum IOCState {
        ANET_TO_BE_CONNECTING = 1,
        ANET_CONNECTING,
        ANET_CONNECTED,
        ANET_CLOSING,
        ANET_CLOSED,
    };

public:
    /**
     * @param owner: pointer of a transport object
     * @param socket pointer of a socket object
     */
    IOComponent(Transport *owner, Socket *socket);

    virtual ~IOComponent();

    /**
     * @param isServer indicate if this component is used as server
     * @return 
     */
    virtual bool init(bool isServer = false) = 0;

    /**
     * create a connection based on this iocomponent;
     * @param streamer:   streamer used to decode/encode data from/to
     * input/output buffer
     * @param serverAdapter:  user defined adapter to serve requests
     */ 
    virtual Connection *createConnection(IPacketStreamer *streamer,
            IServerAdapter *adapter = NULL) {
        return NULL;
    }
    /**
     * 关闭
     */
    virtual void close() {}

    virtual void closeSocket();
    virtual void closeSocketNoLock();

    virtual void closeConnection(Connection *);

    /**
     * 当有数据可写到时被Transport调用
     * 
     * @return 是否成功, true - 成功, false - 失败。
     */
    virtual bool handleWriteEvent() = 0;

    /**
     * 当有数据可读时被Transport调用
     * 
     * @return 是否成功, true - 成功, false - 失败。
     */
    virtual bool handleReadEvent() = 0;

    /***
     * called when error occured
     */
    virtual bool handleErrorEvent() = 0;

    /**
     * 超时检查
     * 
     * @param    now 当前时间(单位us)
     * @return when return false, we should remove this iocomponent
     * from timeout thread's checking list
     */
    virtual bool checkTimeout(int64_t now) = 0;

    /**
     * 得到socket句柄
     * 
     * @return Socket
     */
    inline Socket *getSocket() {
        return _socket;
    }

    /**
     * 设置SocketEvent
     */
    void setSocketEvent(SocketEvent *socketEvent) ;

    /**
     * 设置能读
     *
     * @param on 读是否打开
     */
    void enableRead(bool on) ;

    /**
     * 设置能写
     *
     * @param on 写是否打开
     */
    void enableWrite(bool on) ;

    inline void updateUseTime(int64_t now) {
        _lastUseTime = now;
    }

    /**
     * 增加引用计数
     */
    void addRef() ;

    /**
     * 减少引用计数 
     */
    void subRef();

    /**
     * 取出引用计数 
     */
    int getRef();
    /**
     * 是否连接状态, 包括正在连接状态
     */
    inline bool isConnectState() {
        return getState() < ANET_CLOSING;
    }

    inline bool isClosed() {
        return getState() >= ANET_CLOSING;
    }

    /**
     * 设置是否重连
     */
    inline void setAutoReconn(bool on) {
        /***only attempts to reconnect 30 times at most*/
        _autoReconn = (_isServer || !on) ? 0 : MAX_RECONNECTING_TIMES;
    }
 
    /**
     * 得到重连标志 
     */
    inline bool isAutoReconn() {
        return (_autoReconn && !_isServer);
    }

    /***
     * Is this IOComponent referenced by read/write thread of transport
     */
    bool isReferencedByReadWriteThread() {
        return _refByRreadWriteThread;
    }

    /***
     * Set _refByRreadWriteThread flag
     */
    inline void referencedByReadWriteThread(bool flag) {
        _refByRreadWriteThread = flag;
    }

    void lock() ;
    void unlock();
    
    inline IOCState getState() {
        return _state;
    }

    virtual bool setState(IOCState state) ;
protected:
    Transport *_owner;
    Socket *_socket;    // 一个Socket的文件句柄
    SocketEvent *_socketEvent;
    IOCState _state;         // 连接状态
    atomic_t _refcount; // 引用计数
    int _autoReconn;   // 是否重连
    bool _isServer;     // 是否为服务器端
    int64_t _lastUseTime;   // 最近使用的系统时间    
    bool _enableRead;
    bool _enableWrite;
    bool _refByRreadWriteThread;
    ThreadMutex _mutex;
private:
    IOComponent *_prev; // 用于链表
    IOComponent *_next; // 用于链表
};
}

#endif /*IOCOMPONENT_H_*/
