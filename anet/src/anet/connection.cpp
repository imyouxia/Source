#include <anet/timeutil.h>
#include <anet/connection.h>
#include <anet/streamingcontext.h>
#include <anet/channel.h>
#include <anet/channelpool.h>
#include <anet/ipacketstreamer.h>
#include <anet/iserveradapter.h>
#include <anet/socket.h>
#include <anet/iocomponent.h>
#include <anet/log.h>
#include <stdint.h>
#include <anet/threadmutex.h>

namespace anet {

Connection::Connection(Socket *socket, IPacketStreamer *streamer, 
                       IServerAdapter *serverAdapter) 
{
    assert(streamer);
    _socket = socket;
    _streamer = streamer;
    _serverAdapter = serverAdapter;
    _defaultPacketHandler = NULL;
    _iocomponent = NULL;
    _queueTimeout = 5000;
    _waitingThreads = 0;
    _closed = false;
    _queueLimit = _streamer->existPacketHeader() ? 50 : 1;
    _context = _streamer->createContext();
    _packetPostedCount = 0u;
    _packetHandledCount = 0u;
 }

/*
 * 析构函数
 */
Connection::~Connection() {
    if (!isClosed()) {
        closeHook();
    }
    delete _context;
}

void Connection::close() {
    _iocomponent->closeConnection(this);
}

bool Connection::isClosed() {
    return _closed;
}
/*
 * 发送packet到发送队列
 */
bool Connection::postPacket(Packet *packet, IPacketHandler *packetHandler,
                            void *args, bool blocking) {
    ANET_LOG(SPAM,"postPacket() through IOC(%p).",_iocomponent);

    if (NULL == packet) {
        return false;
    }

    // 如果是client, 并且有queue长度的限制
    _outputCond.lock();
    if (isClosed()) {
        ANET_LOG(DEBUG,"Connection closed! IOC(%p).",_iocomponent);
        _outputCond.unlock();
        return false;
    }
    if (!_isServer && _queueLimit > 0) {//@fix bug #97
        if (!blocking && _channelPool.getUseListCount() >= _queueLimit) {
            _outputCond.unlock();
            ANET_LOG(DEBUG,"QUEUE FULL NO BLOCKING! IOC(%p).",_iocomponent);
            return false;//will not blocking
        }
        while (_channelPool.getUseListCount() >= _queueLimit && !isClosed()) {
            _waitingThreads ++;
            ANET_LOG(SPAM,"Before Waiting! IOC(%p).",_iocomponent);
            _outputCond.wait();
            ANET_LOG(SPAM,"After Waiting! IOC(%p).",_iocomponent);
            _waitingThreads --;
        }
        if (isClosed()) {
            ANET_LOG(DEBUG,"Waked up on closing! IOC(%p).",_iocomponent);
            _outputCond.unlock();
            return false;
        }
    }
    
    if (_isServer) {
        if(_streamer->existPacketHeader()) {
            assert(packet->getChannelId());
        }
    } else {
        uint32_t chid = _streamer->existPacketHeader() 
                        ? 0
                        : ChannelPool::HTTP_CHANNLE_ID;
        Channel *channel = _channelPool.allocChannel(chid);
        if (NULL == channel) {
            ANET_LOG(WARN, "Failed to allocate a channel");
            _outputCond.unlock(); 
            return false;
        }
        channel->setHandler(packetHandler);
        channel->setArgs(args);
        packet->setChannel(channel);
    }
        
    packet->setExpireTime(_queueTimeout);   // 设置超时only client

    // 写入到outputqueue中
    int qsize = _outputQueue.size();
    _outputQueue.push(packet);
    if (qsize==0) {
        ANET_LOG(DEBUG,"IOC(%p)->enableWrite(true)", _iocomponent);
        _iocomponent->enableWrite(true);
    }
    _packetPostedCount++;
    _outputCond.unlock();

    ANET_LOG(DEBUG,"postPacket() Success! IOC(%p).",_iocomponent);
    return true;
}

class SynStub : public IPacketHandler {
public:
    HPRetCode handlePacket(Packet *packet, void *args) {
        MutexGuard guard(&_cond);
        assert(this == args);
        _reply = packet;
        _cond.signal();
	return FREE_CHANNEL;
    }

    Packet* waitReply() {
        _cond.lock();
        while (NULL == _reply) {
            _cond.wait();
        }
        _cond.unlock();
        return _reply;
    }

    SynStub() {
        _reply = NULL;
    }
    virtual ~SynStub() {}
private:
    ThreadCond _cond;
    Packet *_reply;
};

Packet *Connection::sendPacket(Packet *packet) {
    SynStub stub;
    if (_isServer || !postPacket(packet, &stub, &stub)) {
        return NULL;
    }
    return stub.waitReply();
}

/*
 * handlePacket 数据
 */
bool Connection::handlePacket(Packet *packet) {
    IPacketHandler::HPRetCode rc;
    void *args = NULL;
    Channel *channel = NULL;
    IPacketHandler *packetHandler = NULL;

    if (_isServer) {
        beforeCallback();
        rc = _serverAdapter->handlePacket(this, packet);
        afterCallback();
        return true;
    }

    if (!_streamer->existPacketHeader()) {
        packet->setChannelId(ChannelPool::HTTP_CHANNLE_ID);
    }

    uint32_t chid = packet->getChannelId();
    channel = _channelPool.findChannel(chid);
    if (channel == NULL) {
        ANET_LOG(WARN, "No channel found, id: %u", chid);
        packet->free();
        return false;
    }
    packetHandler = channel->getHandler();
    args = channel->getArgs();
    _channelPool.freeChannel(channel);  //fix bug 137
    if (packetHandler == NULL) {
        packetHandler = _defaultPacketHandler;
    }
    assert(packetHandler);
    beforeCallback();
    rc = packetHandler->handlePacket(packet, args);
    afterCallback();
    wakeUpSender();
    return true;
}

/*
 * 检查超时
 */
bool Connection::checkTimeout(int64_t now) {
    ANET_LOG(SPAM, "check Timeout  IOC(%p), CONN(%p)", _iocomponent, this);
    // 得到超时的channel的list
    Channel *list = NULL;
    Channel *channel = NULL;
    IPacketHandler *packetHandler = NULL;
    Packet *cmdPacket = (now == TimeUtil::MAX)
                            ? &ControlPacket::ConnectionClosedPacket
                            : &ControlPacket::TimeoutPacket;

    if (!_isServer && (list = _channelPool.getTimeoutList(now))) {
        ANET_LOG(SPAM,"Checking channel pool. IOC(%p), CONN(%p)", _iocomponent, this);
        channel = list;
        while (channel != NULL) {
            packetHandler = channel->getHandler();
            if (packetHandler == NULL) {    // 用默认的
                packetHandler = _defaultPacketHandler;
            }
            if (packetHandler != NULL) {
                beforeCallback();
                packetHandler->handlePacket(cmdPacket, channel->getArgs());
                afterCallback();
            }
            channel = channel->getNext();
        }
        _channelPool.appendFreeList(list);
    }

    _outputCond.lock();
    ANET_LOG(SPAM,"Checking output queue(%d). IOC(%p), CONN(%p).",
             _outputQueue.size(), _iocomponent, this);
    Packet *packetList = _outputQueue.getTimeoutList(now);
    _outputCond.unlock();
    while (packetList) {
        ANET_LOG(SPAM, "Packet(%p) IOC(%p), CONN(%p)", packetList,
                 _iocomponent, this);
        ANET_LOG(WARN, "Packet Timeout. Now(%ld), Exp(%ld), timeout(%d)",
                 now, packetList->getExpireTime(), _queueTimeout);
        Packet *packet = packetList;
        packetList = packetList->getNext(); 
        if (_isServer) {
            if (_serverAdapter) {
                beforeCallback();                
                _serverAdapter->handlePacket(this, cmdPacket);
                afterCallback();
            }
            packet->free();
            continue;
        }
        channel = packet->getChannel();
        if (channel) {
            packetHandler = channel->getHandler();
            if (packetHandler == NULL) {    // 用默认的
                packetHandler = _defaultPacketHandler;
            }
            if (packetHandler != NULL) {
                beforeCallback();
                packetHandler->handlePacket(cmdPacket, channel->getArgs());
                afterCallback();
            }
            _channelPool.freeChannel(channel);//~Bug #93 
        }
        packet->free();
    }

    wakeUpSender();
    return true;
}

void Connection::closeHook() {
    _outputCond.lock();
    _closed = true;
    _outputCond.unlock();
    checkTimeout(TimeUtil::MAX);
}

void Connection::wakeUpSender() {
    if  (_queueLimit > 0) {
        _outputCond.lock();
        size_t queueTotalSize = _channelPool.getUseListCount();
        if (queueTotalSize < _queueLimit || isClosed()) {
            if (_waitingThreads) {
                _outputCond.broadcast();
            }
        }
        _outputCond.unlock();
    }
}

void Connection::beforeCallback() {
    _iocomponent->unlock();
}

void Connection::afterCallback() {
    _iocomponent->lock();
    _packetHandledCount++;
}

void Connection::addRef() {
    _iocomponent->addRef();
}

void Connection::subRef() {
    _iocomponent->subRef();
}

int Connection::getRef() {
    return _iocomponent->getRef();
}

bool Connection::setQueueLimit(size_t limit) {
    if (_streamer->existPacketHeader()) {
         _queueLimit = limit;
         return true;
    }
    if (limit != 1) {
        return false;
    } else {
        return  true;
    }
}

size_t Connection::getQueueLimit() {
    return _queueLimit;
}

void Connection::resetContext() {
    clearOutputBuffer();
    clearInputBuffer();
    _context->reset();
}

}
