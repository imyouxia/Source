#include <anet/log.h>
#include <anet/stats.h>
#include <anet/streamingcontext.h>
#include <anet/channel.h>
#include <anet/channelpool.h>
#include <anet/socket.h>
#include <anet/ipacketstreamer.h>
#include <anet/iserveradapter.h>
#include <anet/tcpconnection.h>
#include <anet/iocomponent.h>
namespace anet {

TCPConnection::TCPConnection(Socket *socket, IPacketStreamer *streamer,
                             IServerAdapter *serverAdapter) : Connection(socket, streamer, serverAdapter) {
    _gotHeader = false;
    _writeFinishClose = false;
    memset(&_packetHeader, 0, sizeof(_packetHeader));
}

TCPConnection::~TCPConnection() {
    ;
}

bool TCPConnection::writeData() {
    //to reduce the odds of blocking postPacket() 
    _outputCond.lock();
    _outputQueue.moveTo(&_myQueue);
    if (_myQueue.size() == 0 && _output.getDataLen() == 0) { // 返回
        ANET_LOG(DEBUG,"IOC(%p)->enableWrite(false)",_iocomponent);
        _iocomponent->enableWrite(false);
        _outputCond.unlock();
        return true;
    }
    _outputCond.unlock();

    Packet *packet;
    int ret = 0;
    int writeCnt = 0;
    int myQueueSize = _myQueue.size();
    int error = 0;

    do {
        // 写满到
        while (_output.getDataLen() < READ_WRITE_SIZE) {
            // 队列空了就退出

            if (myQueueSize == 0)
                break;

            packet = _myQueue.pop();
            myQueueSize --;
            _streamer->encode(packet, &_output);
            Channel *channel = packet->getChannel();
            if (channel) {
                channel->setExpireTime(packet->getExpireTime());
                if (_defaultPacketHandler == NULL 
                    && channel->getHandler() == NULL) {
                    //free channle of packets not expecting reply
                    _channelPool.freeChannel(channel);
                }
            }
            packet->free();
            ANET_COUNT_PACKET_WRITE(1);
        }

        if (_output.getDataLen() == 0) {
            break;
        }

        // write data
        ret = _socket->write(_output.getData(), _output.getDataLen());
        if (ret > 0) {
            _output.drainData(ret);
        } else {
            error = _socket->getSoError();
        }

        writeCnt ++;
    } while (ret > 0 && _output.getDataLen() == 0 
             /**@todo remove magic number 10*/
             && myQueueSize>0 && writeCnt < 10);

    _output.shrink();

    _outputCond.lock();
    _outputQueue.moveBack(&_myQueue);

    if (error !=0 && error != EWOULDBLOCK && error != EAGAIN) {
        char spec[32];
        ANET_LOG(DEBUG,"Connection (%s) write error: %d", 
                 _socket->getAddr(spec, 32), error);
        _outputCond.unlock();
        return false;
    }
    int queueSize = _outputQueue.size() + (_output.getDataLen() > 0 ? 1 : 0);
    if (queueSize > 0) {
//when using level triggered mode, do NOT need to call enableWrite() any more.
//        ANET_LOG(DEBUG,"IOC(%p)->enableWrite(true)", _iocomponent);
//        _iocomponent->enableWrite(true);
    } else if (_writeFinishClose) {
        ANET_LOG(DEBUG, "Initiative cut connect.");
        _outputCond.unlock();
        return false;
    } else {
        ANET_LOG(DEBUG,"IOC(%p)->enableWrite(false)", _iocomponent);
        _iocomponent->enableWrite(false);
    }

    // 如果是client, 并且有queue长度的限制
    if (!_isServer && _queueLimit > 0) {
        size_t queueTotalSize = _channelPool.getUseListCount();
        if (queueTotalSize < _queueLimit && _waitingThreads > 0) {
            _outputCond.broadcast();
        }
    }
    _outputCond.unlock();

    return true;
}

bool TCPConnection::readData() {
    int ret = 0;
    int readCnt = 0;
    bool broken = false;
    int error = 0;

    do {
        readCnt++;
        _input.ensureFree(READ_WRITE_SIZE);
        ret = _socket->read(_input.getFree(), READ_WRITE_SIZE);
        ANET_LOG(SPAM,"%d bytes read", ret);
        if (ret < 0) {
            error = _socket->getSoError();
            break;
        }
        if (0 == ret) {
            _context->setEndOfFile(true);
        }
        _input.pourData(ret);
        while (_streamer->processData(&_input, _context)) {
            if (!_context->isCompleted()) {
                break;
            }
            Packet *packet = _context->stealPacket();
            handlePacket(packet);
            _context->reset();
            ANET_COUNT_PACKET_READ(1);
            
        }
        broken = _context->isBroken();
    } while (ret == READ_WRITE_SIZE && !broken && readCnt < 10);

    _input.shrink();
    if (!broken) {
        if (ret == 0) {
            broken = true;
        } else if (ret < 0) {
            if (error != 0 && error != EAGAIN && error != EWOULDBLOCK) {
                char spec[32];
                ANET_LOG(DEBUG,"Connection (%s) read error: %d",
                        _socket->getAddr(spec, 32), error);
                broken = true;
            }
        }
    }

//when using level triggered mode, do NOT need to call enableRead() any more.
//     if (!broken) {
//         _outputCond.lock();
//         ANET_LOG(DEBUG,"IOC(%p)->enableRead(true)", _iocomponent);
//         _iocomponent->enableRead(true);
//         _outputCond.unlock();
//     }

    return !broken;
}
}
