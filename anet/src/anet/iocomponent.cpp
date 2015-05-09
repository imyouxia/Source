#include <anet/timeutil.h>
#include <anet/socket.h>
#include <anet/socketevent.h>
#include <anet/log.h>
#include <anet/iocomponent.h>
#include <anet/connection.h>
namespace anet {

/*
 * 构造函数
 */
IOComponent::IOComponent(Transport *owner, Socket *socket) {
    assert(socket);
    assert(owner);
    _owner = owner;
    _socket = socket;
    _socket->setIOComponent(this);
    _socketEvent = NULL;
    atomic_set(&_refcount, 1);
    _state = ANET_TO_BE_CONNECTING; // 正在连接
    _autoReconn = 0; // 不要自动重连
    _prev = _next = NULL;
    _lastUseTime = TimeUtil::getTime();
    _enableRead = true;
    _enableWrite = false;
    _refByRreadWriteThread = false;
}

/*
 * 析构函数
 */
IOComponent::~IOComponent() {
    if (_socket) {
        _socket->close();
        delete _socket;
        _socket = NULL;
    }
}

void IOComponent::setSocketEvent(SocketEvent *socketEvent) {
    _socketEvent = socketEvent;
}

void IOComponent::enableRead(bool on) {
    _enableRead = on;
    bool read = _enableRead;
    bool write = _enableWrite;
    if (_socketEvent) {
        ANET_LOG(SPAM,"setEvent(R:%d,W:%d). IOC(%p)", read, write, this);
        _socketEvent->setEvent(_socket, read, write);
    }
}

void IOComponent::enableWrite(bool on) {
    _enableWrite = on;
    bool read = _enableRead;
    bool write = _enableWrite;
    if (_socketEvent) {
        ANET_LOG(SPAM,"setEvent(R:%d,W:%d). IOC(%p)", read, write, this);
        _socketEvent->setEvent(_socket, read, write);
    }
}

void IOComponent::addRef() {
    ANET_LOG(SPAM,"IOC(%p)->addRef(), [%d]", this, _refcount.counter);
    atomic_add(1, &_refcount);
}

void IOComponent::subRef() {
    ANET_LOG(SPAM,"IOC(%p)->subRef(), [%d]", this, _refcount.counter);
    int ref = atomic_dec_return(&_refcount);
    if (!ref) {
    ANET_LOG(SPAM,"Deleting this IOC(%p)", this);
        delete this;
    }
}

void IOComponent::closeConnection(Connection *conn) {
    conn->closeHook();
}

int IOComponent::getRef() {
    return atomic_read(&_refcount);
}

void IOComponent::lock() {
    _mutex.lock();
}

void IOComponent::unlock() {
    _mutex.unlock();
}

bool IOComponent::setState(IOCState state) {
    _state = state;
    return true;
}

void IOComponent::closeSocketNoLock() {
   if (_socket) {
       _socketEvent->removeEvent(_socket);
       _socket->close();
    }
}

void IOComponent::closeSocket() {
    lock();
    closeSocketNoLock();
    unlock();
}

}
