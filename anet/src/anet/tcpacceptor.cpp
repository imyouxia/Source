#include <anet/transport.h>
#include <anet/serversocket.h>
#include <anet/socket.h>
#include <anet/tcpacceptor.h>
#include <anet/tcpcomponent.h>
#include <anet/log.h>

namespace anet {
/**
 * 构造函数，由Transport调用。
 *
 * @param  owner:    运输层对象
 * @param  host:   监听ip地址或hostname
 * @param port:   监听端口
 * @param streamer:   数据包的双向流，用packet创建，解包，组包。
 * @param serverAdapter:  用在服务器端，当Connection初始化及Channel创建时回调时用
 */
TCPAcceptor::TCPAcceptor(Transport *owner, Socket *socket,
                         IPacketStreamer *streamer, 
                         IServerAdapter *serverAdapter,
                         int timeout) : IOComponent(owner, socket) {
    _streamer = streamer;
    _serverAdapter = serverAdapter;
    _timeout = timeout;
}

/*
 * 初始化, 开始监听
 */
bool TCPAcceptor::init(bool isServer) {
    setSocketEvent(_owner->getSocketEvent());
    assert(_socketEvent);
    _socket->setSoBlocking(false);
    bool rc = ((ServerSocket*)_socket)->listen();
    if (rc) {
        setState(ANET_CONNECTED);//may be need new state
        _socketEvent->addEvent(_socket, true,false);
        _owner->postCommand(Transport::TC_ADD_IOC, this);
    }
    return rc;
}

/**
* 当有数据可读时被Transport调用
*
* @return 是否成功
*/
bool TCPAcceptor::handleReadEvent() {
    lock();
    Socket *socket;
    while ((socket = ((ServerSocket*)_socket)->accept()) != NULL) {
        ANET_LOG(DEBUG, "New connection coming. fd=%d", socket->getSocketHandle());
        TCPComponent *component = new TCPComponent(_owner, socket);
        assert(component);

         if (!component->init(true)) {
             delete component;/**@TODO: may coredump?*/
             return true;
        }
        Connection *conn = component->createConnection(_streamer, _serverAdapter);
        conn->setQueueTimeout(_timeout);
         _owner->addToCheckingList(component);

        //transport's Read Write Thread and Timeout thread will have their
        //own reference of component, so we need to subRef() the initial one
        component->subRef();
    }
    unlock();
    return true;
}

bool TCPAcceptor::handleErrorEvent() {
    close();
    return false;
}

void TCPAcceptor::close() {
    lock();
    if (getState() != ANET_CLOSED) {
        _socket->close();
        setState(ANET_CLOSED);
    }
    _owner->postCommand(Transport::TC_REMOVE_IOC, this);
    unlock();
}

/*
 * 超时检查
 *
 * @param    now 当前时间(单位us)
 */
bool TCPAcceptor::checkTimeout(int64_t now) {return true;}
}
