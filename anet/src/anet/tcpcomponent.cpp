#include <anet/timeutil.h>
#include <anet/transport.h>
#include <anet/socket.h>
#include <anet/tcpcomponent.h>
#include <anet/tcpconnection.h>
#include <anet/log.h>

namespace anet {
TCPComponent::TCPComponent(Transport *owner, Socket *socket)
    : IOComponent(owner, socket) 
{
    _startConnectTime = 0;
    _isServer = false;
    _connection = NULL;
    setSocketEvent(_owner->getSocketEvent());
    assert(_socketEvent);
}

TCPComponent::~TCPComponent() {
    if (_connection) {
        ANET_LOG(DEBUG,"delete connection(%p)",_connection);
        delete _connection;
        _connection = NULL;
    }
}

bool TCPComponent::init(bool isServer) {
    _socket->setSoLinger(false, 0);
    _socket->setIntOption(SO_KEEPALIVE, 1);
    _socket->setTcpNoDelay(true);
    if (!isServer) {
        if (!socketConnect()) {
            return false;
        }
        _enableWrite = true;
        setState(ANET_CONNECTING);
    } else  {
        _socket->setSoBlocking(false);
        _enableWrite = false;
        setState(ANET_CONNECTED);
    }
	int64_t curTime = TimeUtil::getTime();
    updateUseTime(curTime);
	//add assignment statement for _startConnectTime when init TCPComponent for Ticket #23 by wanggf 200810162042
	_startConnectTime = curTime;
    _enableRead = true;
    _isServer = isServer;

    return true;
}

Connection* TCPComponent::createConnection(IPacketStreamer *streamer,
        IServerAdapter *adapter) {
    if (NULL == streamer) {
        return NULL;
    }
    if (_isServer && NULL == adapter) {
        return NULL;
    }
        
    _connection = new TCPConnection(_socket, streamer, adapter);
    assert(_connection);
    _connection->setIOComponent(this);
    _connection->setServer(_isServer);

    _socketEvent->addEvent(_socket, _enableRead, _enableWrite);
    _owner->postCommand(Transport::TC_ADD_IOC, this);
    return _connection;
}

bool TCPComponent::socketConnect() {
    if (!_socket->isAddressValid()) {
        return false;
    }
    _socket->setSoBlocking(false);
    if (!_socket->connect()) {
        char tmp[32];
        int error = Socket::getLastError();
        if (error != EINPROGRESS && error != EWOULDBLOCK) {
            _socket->close();
            ANET_LOG(ERROR, "failed to connect to %s, %s(%d)",
                     _socket->getAddr(tmp, 32), strerror(error), error);
            return false;
        }
    }
    return true;
}

/**
 * @todo: redefine close()
 */
void TCPComponent::close() {
    closeConnection(_connection);
}

void TCPComponent::closeConnection(Connection *conn) {
    assert(_connection == conn);
    lock();
    closeSocketNoLock();
    if ( setState(ANET_CLOSING) ) {
        _owner->postCommand(Transport::TC_REMOVE_IOC, this);
    }
    unlock();
}

// void TCPComponent::closeSocketNoLock() {
//    if (_socket) {
//        _socketEvent->removeEvent(_socket);
//        _socket->close();
//     }
// }

// void TCPComponent::closeSocket() {
//     lock();
//     closeSocketNoLock();
//     unlock();
// }

bool TCPComponent::handleWriteEvent() {
    lock();
    bool rc = true;
    ANET_LOG(DEBUG,"(IOC:%p)", this);
    if (getState() == ANET_CONNECTING) {
        ANET_LOG(SPAM,"Write ANET_CONNECTING(IOC:%p)", this);
        setState(ANET_CONNECTED);
        setAutoReconn(isAutoReconn());
        _connection->resetContext();
    }

    if (getState() == ANET_CONNECTED) {
        ANET_LOG(SPAM,"Write ANET_CONNECTED (IOC:%p)", this);
        rc = _connection->writeData();
        if (!rc) {
            closeSocketNoLock();
            setState(_autoReconn ? ANET_TO_BE_CONNECTING : ANET_CLOSING);
            _owner->postCommand(Transport::TC_REMOVE_IOC, this);
        }
    } else {
        rc = false;
        ANET_LOG(WARN, "State(%d) changed by other thread!", getState());
    }
    unlock();
    return rc;
}

bool TCPComponent::handleErrorEvent() {
    lock();
    ANET_LOG(DEBUG,"(IOC:%p)", this);
	//add stat ANET_CONNECTING when handle error for Ticket #23 by wanggf 200810162037
    if ((getState() == ANET_CONNECTED) || (getState() == ANET_CONNECTING)) {
        ANET_LOG(DEBUG,"Error IOCState:%d (IOC:%p)", getState(), this);
        closeSocketNoLock();
        setState(_autoReconn ? ANET_TO_BE_CONNECTING : ANET_CLOSING);
        _owner->postCommand(Transport::TC_REMOVE_IOC, this);
    }
    unlock();
    return false;
}

bool TCPComponent::handleReadEvent() {
    lock();
    ANET_LOG(DEBUG,"(IOC:%p)",this);
    bool rc = true;
    if (getState() == ANET_CONNECTED) {
        ANET_LOG(DEBUG,"Read ANET_CONNECTED (IOC:%p)", this);
        rc = _connection->readData();
        if (!rc) {
            closeSocketNoLock();
            setState(_autoReconn ? ANET_TO_BE_CONNECTING : ANET_CLOSING);
            _owner->postCommand(Transport::TC_REMOVE_IOC, this);
        }
    } else {
        rc = false;
        ANET_LOG(WARN, "State(%d) changed by other thread!", getState());
    }
    unlock();
    return rc;
}

/**
 * checking time out 
 *
 * @param    now 
 * @return if return false, we should remove this iocomponent
 * from timeout thread's checking list
 */
bool TCPComponent::checkTimeout(int64_t now) {
    lock();
    char tmp[32];
    bool rc = true;

    if (getState() < ANET_CLOSING) {
        _connection->checkTimeout(now);
    } else {
        _connection->closeHook();
    }

    if (getState() == ANET_CONNECTING) {
        if (_startConnectTime > 0 
            && _startConnectTime < (now - CONNECTING_TIMEOUT)) {
            ANET_LOG(ERROR, "Connecting to %s timeout.", _socket->getAddr(tmp, 32));
            closeSocketNoLock();
            setState(_autoReconn ? ANET_TO_BE_CONNECTING : ANET_CLOSING);
            _owner->postCommand(Transport::TC_REMOVE_IOC, this);
            _startConnectTime = now;
        }
    } else if (getState() == ANET_CONNECTED && _isServer) { 
        int64_t idle = now - _lastUseTime;
        if (idle > MAX_IDLE_TIME) { 
            _autoReconn = 0;
            closeSocketNoLock();
            setState(ANET_CLOSING);
            _owner->postCommand(Transport::TC_REMOVE_IOC, this);
            ANET_LOG(WARN, "Closing idle connection. %s (idle time: %d secs IOC:%p).",
                     _socket->getAddr(tmp, 32), (idle/static_cast<int64_t>(1000000)), this);
        }
    } else if (getState() == ANET_TO_BE_CONNECTING && _autoReconn && 
            _startConnectTime < now - RECONNECTING_INTERVAL ) { 
        ANET_LOG(INFO, "Reconnecting: %s", _socket->getAddr(tmp, 32));
        _autoReconn -- ;
        _startConnectTime = now;
        if (socketConnect()) {
            assert(_socketEvent);
            _enableRead = _enableWrite = true;
            _socketEvent->addEvent(_socket, _enableRead, _enableWrite);
            setState(ANET_CONNECTING);
            _owner->postCommand(Transport::TC_ADD_IOC, this);
        } else if (!_autoReconn) {
            ANET_LOG(ERROR, "exceeding MAX_RECONNECTING_TIMES!");
            setState(ANET_CLOSING);
        }
    } else if ( getState() == ANET_CLOSING &&  getRef() == 1 ) {
        setState(ANET_CLOSED);
        rc = false;
    }

    unlock();
    return rc;
}

bool TCPComponent::setState(IOCState state) {
    if (_state < ANET_CLOSING) {
        _state = state;
        if (_connection && _state >= ANET_CLOSING) {
            _connection->closeHook();
        }
        ANET_LOG(SPAM,"IOC(%p) state: %d", this, _state);
        return true;
    }
    if (state > _state) {
        _state = state;
        ANET_LOG(SPAM,"IOC(%p) state: %d", this, _state);
        return true;
    }
    ANET_LOG(SPAM,"IOC(%p) state: %d(NOT CHANGED)", this, _state);
    return false;
}

}
