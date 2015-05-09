#include "connectiontf.h"
#include <anet/tcpconnection.h>
#include <anet/tcpcomponent.h>
#include <anet/channel.h>
#include "testadapter.h"
#include <anet/iserveradapter.h>
#include <anet/ipackethandler.h>
#include <anet/runnable.h>
#include <anet/defaultpacketfactory.h>
#include <anet/httppacket.h>
#include <anet/httpstreamer.h>
#include <anet/httppacketfactory.h>

using namespace std;

namespace anet {
class EmptyServerAdapter : public IServerAdapter
{
public:
    IPacketHandler::HPRetCode handlePacket(Connection *connection, Packet *packet)
    {
        packet->free();
        return IPacketHandler::FREE_CHANNEL;
    }    
};    


int ConnPacket::_destructNum = 0;

CPPUNIT_TEST_SUITE_REGISTRATION(ConnectionTF);

ConnectionTF::ConnectionTF() {
    _factory = new ConnPacketFactory;
    assert(_factory);
    _streamer = new DefaultPacketStreamer(_factory);
    assert(_streamer);
    _handler = new DefaultPacketHandler();
    assert(_handler);
}

ConnectionTF::~ConnectionTF() {
    delete _streamer;
    delete _factory;
    delete _handler;
}

void ConnectionTF::setUp() { 
    CPPUNIT_ASSERT(_component = new ConnComponent(new Socket));
    _serverAdapter = new EmptyServerAdapter;
    _connection = new ConnectionForTest( _component->getSocket(),_streamer, _serverAdapter);
    CPPUNIT_ASSERT(_connection);
    _connection->setIOComponent(_component);
    _connection->setServer(false);
}

void ConnectionTF::tearDown() { 
    if (_connection) {
        delete _connection;
        _connection = NULL;
    }
    if (_component) {
        delete _component;
        _component = NULL;
    }
    if (_serverAdapter) {
        delete _serverAdapter;
        _serverAdapter = NULL;
    }
}
void ConnectionTF::testPostPacket() {
    ConnPacket *packet = new ConnPacket;

    _connection->closeHook();
    CPPUNIT_ASSERT(!_connection->postPacket(packet, NULL, NULL));

    _connection->_closed = false;
    CPPUNIT_ASSERT(!_connection->postPacket(NULL, NULL, NULL));
    CPPUNIT_ASSERT(_connection->postPacket(packet, NULL, NULL));
    CPPUNIT_ASSERT(packet->getChannel());
    CPPUNIT_ASSERT(!packet->getChannel()->getHandler());
    CPPUNIT_ASSERT_EQUAL((size_t)1, _connection->_outputQueue.size());

    packet = new ConnPacket();
    CPPUNIT_ASSERT(_connection->postPacket(packet, NULL, NULL));
    CPPUNIT_ASSERT_EQUAL((size_t)2, _connection->_outputQueue.size());

    packet = new ConnPacket();
    CPPUNIT_ASSERT(_connection->postPacket(packet, NULL, NULL));
    CPPUNIT_ASSERT_EQUAL((size_t)3, _connection->_outputQueue.size());

    //test lock
    //int queueLimit = _connection->_queueLimit;
    while (_connection->_channelPool.getUseListCount() 
           < _connection->_queueLimit) {
        packet = new ConnPacket();
        CPPUNIT_ASSERT(_connection->postPacket(packet, NULL, NULL));
    }
    packet = new ConnPacket();
    CPPUNIT_ASSERT(!_connection->postPacket(packet, NULL, NULL));
    delete packet;
    CPPUNIT_ASSERT_EQUAL(_connection->_queueLimit, 
                         _connection->_outputQueue.size());
      
    class ShouldBlocking : public Runnable
    {
    public:
        void run(Thread *thread, void *args)
        {
            Connection *conn = static_cast<Connection *>(args);
            ConnPacket *packet = new ConnPacket();
            packet->setString("test");
            _cond.lock();
            _cond.signal();
            _cond.unlock();
            _startTime = TimeUtil::getTime();
            _postPacket = conn->postPacket(packet, NULL, NULL,true);
            _endTime = TimeUtil::getTime();
            _timeUsed = _endTime - _startTime;
            ANET_LOG(DEBUG,"Time used for postPacket(): %d us", _timeUsed);
            packet->free();
        }
    public:
        int64_t _timeUsed;
        int64_t _startTime;
        int64_t _endTime;
        bool _postPacket;
        ThreadCond _cond;
    };
    size_t tmpsize = _connection->_outputQueue.size();
    ShouldBlocking blockingAdd;
    Thread thread;

    blockingAdd._cond.lock();
    thread.start(&blockingAdd, _connection);
    blockingAdd._cond.wait();
    blockingAdd._cond.unlock();
    usleep(100000);
    CPPUNIT_ASSERT_EQUAL(tmpsize, _connection->_outputQueue.size());
    int64_t closeTime = TimeUtil::getTime();
    _connection->closeHook();
    CPPUNIT_ASSERT_EQUAL((size_t)0, _connection->_outputQueue.size());
    thread.join();
    CPPUNIT_ASSERT(!blockingAdd._postPacket);
    CPPUNIT_ASSERT(closeTime > blockingAdd._startTime);
    CPPUNIT_ASSERT(closeTime < blockingAdd._endTime);
    CPPUNIT_ASSERT(blockingAdd._timeUsed > 50000);
}
	
void ConnectionTF::testHandlePacket() {
    DataBuffer input;
    Channel *channel;
    ChannelPool *channelPool = &_connection->_channelPool;
    DefaultPacket *packet = new ConnPacket;

    ConnPacket::_destructNum = 0;
    packet->setChannelId(2);
    //No handler found.
    CPPUNIT_ASSERT(!_connection->handlePacket(packet));
    CPPUNIT_ASSERT_EQUAL(1, ConnPacket::_destructNum);

    //use handler in channel
    packet = new ConnPacket;
    channel = channelPool->allocChannel(0);
    packet->setChannelId(channel->getId());
    channel->setHandler(_handler);

    CPPUNIT_ASSERT(_connection->handlePacket(packet));
    CPPUNIT_ASSERT_EQUAL(2, ConnPacket::_destructNum);

    //use default handler
    packet = new ConnPacket;
    _connection->setDefaultPacketHandler(_handler);
    channel = channelPool->allocChannel(0);
    packet->setChannelId(channel->getId());
    channel->setHandler(NULL);
    CPPUNIT_ASSERT(_connection->handlePacket(packet));
    CPPUNIT_ASSERT_EQUAL(3, ConnPacket::_destructNum);
}
	
void ConnectionTF::testCheckTimeout() {
    _connection->setDefaultPacketHandler(_handler);

    //no expiretime
    CPPUNIT_ASSERT(_connection->checkTimeout(TimeUtil::getTime()));

    //check time out in outputqueue
    ChannelPool &channelPool = _connection->_channelPool;
    PacketQueue &outputQueue = _connection->_outputQueue;

    ANET_LOG(SPAM,"Checking timeout packet in outputQueue");
    /**
     * this should be done in testing case PacketQueueTF
     */
    for(int i = 0; i<3; i++) {
        ConnPacket *packet = new ConnPacket;
        CPPUNIT_ASSERT(packet);
        packet->setString("just for test");
        Channel *chann = channelPool.allocChannel(0);
        chann->setHandler(NULL);
        chann->setArgs(NULL);
        packet->setChannel(chann);
        packet->setExpireTime(100); //it's millseconds
        outputQueue.push(packet);
    }
    size_t size  = 5;
    for(size_t i = 0; i<size; i++) {
        ConnPacket *packet = new ConnPacket;
        packet->setString("just for test");
        Channel *chann = channelPool.allocChannel(0);
        chann->setHandler(NULL);
        chann->setArgs(NULL);
        packet->setChannel(chann);
        outputQueue.push(packet);
        packet->setExpireTime(10000); //it's millseconds
    }
    CPPUNIT_ASSERT(_connection->checkTimeout(TimeUtil::getTime() + 101*1000)); //it's microseconds
    CPPUNIT_ASSERT_EQUAL(size, outputQueue.size());
    CPPUNIT_ASSERT_EQUAL(outputQueue.size(),
                         channelPool.getUseListCount());
    _connection->closeHook();
    CPPUNIT_ASSERT_EQUAL((size_t)0, outputQueue.size());

    /**
     * @todo this shoud be done in testing case ChannelPoolTF
     */
    ANET_LOG(SPAM,"checking timeout in channelPool");
    for(int i = 0; i<6; i++) {
        ConnPacket *packet = new ConnPacket;
        packet->setString("just for test");
        Channel *chann = channelPool.allocChannel(0);
        chann->setHandler(NULL);
        chann->setArgs(NULL);
        packet->setChannel(chann);
        outputQueue.push(packet);
        packet->setExpireTime(1000);
    }        
    CPPUNIT_ASSERT_EQUAL((size_t)6, outputQueue.size());
    ANET_LOG(SPAM,"Pop two packets from outputQueue");
    for(int i = 0; i<2; i++) {
        Packet *packet = outputQueue.pop();
        CPPUNIT_ASSERT(packet);
        Channel *channel = packet->getChannel();
        channel->setExpireTime(TimeUtil::getTime() + 100000);
        packet->free();
    }
    CPPUNIT_ASSERT(_connection->checkTimeout(TimeUtil::getTime() + 200000));
    CPPUNIT_ASSERT_EQUAL((size_t)4, channelPool.getUseListCount());
    _connection->closeHook();
}
void ConnectionTF::testCloseHook() {
    ANET_LOG(SPAM, "testCloseHook() start");
    Socket *socket = new Socket;
    ConnPacketFactory factory;
    ConnServerAdapter adapter;
    DefaultPacketStreamer streamer(&factory);
    Transport transport;
    TCPComponent ioc(&transport, socket);
    //ioc.createConnection(&streamer, &adapter);
    DefaultPacketHandler handler;
    TCPConnection *conn = new TCPConnection(socket, &streamer, &adapter);
    conn->setIOComponent(&ioc);
    conn->setServer(false);
    conn->postPacket(new ConnPacket, &handler, NULL, true);
    conn->postPacket(new ConnPacket, &handler, NULL, true);
    conn->closeHook();
    CPPUNIT_ASSERT_EQUAL((size_t)0, conn->_outputQueue.size());
    
    delete conn;
    conn = new TCPConnection(socket, &streamer, NULL);
    conn->setIOComponent(&ioc);    
    conn->setServer(true);
    ConnPacket *packet = new ConnPacket;
    packet->setChannelId(1);
    conn->postPacket(packet, &handler, NULL, true);
    packet = new ConnPacket;
    packet->setChannelId(2);
    conn->postPacket(packet, &handler, NULL, true);
    conn->closeHook();
    CPPUNIT_ASSERT_EQUAL((size_t)0, conn->_outputQueue.size());

    delete conn;
    conn = new TCPConnection(socket, &streamer, NULL);
    conn->setQueueTimeout(0);
    conn->setIOComponent(&ioc);    
    conn->setServer(false);
    packet = new ConnPacket;
    packet->setChannelId(1);
    conn->postPacket(packet, &handler, NULL, true);
    packet = new ConnPacket;
    packet->setChannelId(2);
    conn->postPacket(packet, &handler, NULL, true);
    conn->closeHook();
    CPPUNIT_ASSERT_EQUAL((size_t)0, conn->_outputQueue.size());
    delete conn;
}


void ConnectionTF::testClose() {
    class PostPacket : public Runnable{
    public:
        void run(Thread *thread, void *args) {
            Connection *conn = static_cast<Connection *>(args);
            DefaultPacketHandler handler;
            for(int i = 0; i<4; i++) {
                ConnPacket *packet = new ConnPacket;
                if (!conn->postPacket(packet, &handler, NULL, true))
                    delete packet;
            }
        }
    };

    
    Socket *socket = new Socket;
    ConnPacketFactory factory;
    ConnServerAdapter adapter;
    DefaultPacketStreamer streamer(&factory);
    Transport transport;
    TCPComponent ioc(&transport, socket);
    //ioc.createConnection(&streamer, &adapter);
    DefaultPacketHandler handler;
    TCPConnection *conn = new TCPConnection(socket, &streamer, &adapter);
    ConnPacket::_destructNum = 0;
    conn->setIOComponent(&ioc);
    conn->setQueueLimit(2);
    conn->setServer(false);
    
    Thread thread;
    PostPacket postPacket;
    thread.start(&postPacket, conn);
    usleep(100000);
    conn->closeHook();
    thread.join();
    CPPUNIT_ASSERT_EQUAL(4, ConnPacket::_destructNum);
    delete conn;
}


void ConnectionTF::testTimeoutMultiThreadSafe() {
    ANET_LOG(DEBUG, "Begin testTimeoutMultiThreadSafe()");
    class TimeoutTestingHandler : public IPacketHandler,
                                  public IServerAdapter,
                                  public Runnable  
    {
    public:
        IPacketHandler::HPRetCode 
        handlePacket(Connection *connection, Packet *packet) {
            if (packet->isRegularPacket() && connection->postPacket(packet) ) {
                return IPacketHandler::FREE_CHANNEL;
            }
            packet->free();
            return IPacketHandler::FREE_CHANNEL;
        }
        IPacketHandler::HPRetCode 
        handlePacket(Packet *packet, void *args) {
            uint32_t *id = (uint32_t *) args;
            CPPUNIT_ASSERT_EQUAL(0x12345u, *id);
            if (packet->isRegularPacket()) {
                _condition.lock();
                ANET_LOG(DEBUG, "Reply for request(%d) Received!", *id);
                _clientPacketReceived ++;
                _condition.wait(1000000);//wait 1000 second
                _condition.unlock();
            } else {
                _condition.lock();
                ANET_LOG(DEBUG, "Control Packet(%s) forrequest(%d) Received!", 
                        ((ControlPacket*)packet)->what(), *id);
                _clientControlPacketReceived ++;
                _condition.signal();
                _condition.unlock();
            }
            packet->free();
            return IPacketHandler::FREE_CHANNEL;
        }
        void run(Thread *thread, void *arg) {
            Transport *transport = (Transport*) arg;
            while (!_stop) {//use fake time two mimit transport r/w thread
                transport->eventIteration(_now);
            }
        }
        TimeoutTestingHandler() {
            _serverPacketReceived = 0;
            _serverControlPacketReceived = 0;
            _clientPacketReceived = 0;
            _clientControlPacketReceived = 0;
            _now = TimeUtil::getTime();
            _stop = false;
        }
        uint32_t _serverPacketReceived;
        uint32_t _serverControlPacketReceived;
        uint32_t _clientPacketReceived;
        uint32_t _clientControlPacketReceived;
        ThreadCond _condition;
        int64_t _now;
        bool _stop;
    } handler;

    IPacketFactory *factory = new DefaultPacketFactory();
    IPacketStreamer *streamer = new DefaultPacketStreamer(factory);
    Transport transport;
    Thread thread;
    const char *spec = "tcp:localhost:5555";
    IOComponent *listener 
        = transport.listen(spec, streamer,&handler);
    CPPUNIT_ASSERT(listener);
    Connection *connection 
        = transport.connect(spec, streamer);
    CPPUNIT_ASSERT(connection);
    connection->setQueueTimeout(1000);//set timeout to 1 second
    thread.start(&handler, &transport);//mimit transport r/w thread
    Packet *packet = factory->createPacket(1);
    uint32_t id = 0x12345u;//packet id
    ANET_LOG(DEBUG, "Send request(%d)", id);
    CPPUNIT_ASSERT(connection->postPacket(packet, &handler, &id));//send request
    while (0 == handler._clientPacketReceived) {
        ANET_LOG(SPAM, "Waiting server reply");
        usleep(10000);//wait 
    } 
    ANET_LOG(SPAM, "client got reply");
    ANET_LOG(SPAM, "Make request timeout");
    transport.timeoutIteration(handler._now + 10000000);
    ANET_LOG(SPAM, "timeoutIteration finished");
    handler._condition.signal();
    ANET_LOG(SPAM, "stop fake transport");
    handler._stop = true;
    thread.join();
    ANET_LOG(SPAM, "fake transport stoped");
    connection->subRef();
    listener->subRef();
    delete streamer;
    delete factory;
    CPPUNIT_ASSERT_EQUAL(1u, handler._clientPacketReceived);
    CPPUNIT_ASSERT_EQUAL(0u, handler._clientControlPacketReceived);
    ANET_LOG(DEBUG, "End testTimeoutMultiThreadSafe()");
}

void ConnectionTF::testHTTPHandler() {
    ANET_LOG(DEBUG, "Begin testHTTPHandler()");
    Transport tranServer;
    Transport tranClient;
    int64_t now;
    HTTPPacketFactory factory;
    HTTPStreamer httpStreamer(&factory);
    HTTPHandler handler;    
    char sepc[] = "tcp:localhost:12349";
    IOComponent *listener = tranServer.listen(sepc, &httpStreamer, &handler);
    tranServer.runIteration(now);
    Connection *conn = tranClient.connect(sepc, &httpStreamer);
    tranServer.runIteration(now);
    tranClient.runIteration(now);
    CPPUNIT_ASSERT(tranClient._iocListHead);
    CPPUNIT_ASSERT(tranServer._iocListHead);
    HTTPPacket *packet = new HTTPPacket;
    packet->setPacketType(HTTPPacket::PT_REQUEST);
    packet->setMethod(HTTPPacket::HM_GET);
    packet->setVersion(HTTPPacket::HTTP_1_1);
    packet->setURI("/abc");
    void *args = (void *)0x12345678;
    CPPUNIT_ASSERT(conn->postPacket(packet, &handler, args, false));
    ANET_LOG(SPAM, "client postpacket");
    tranClient.runIteration(now);
    ANET_LOG(SPAM, "server get packet");
    tranServer.runIteration(now);
    ANET_LOG(SPAM, "server post response");
    tranServer.runIteration(now);
    ANET_LOG(SPAM, "client receive response");
    tranClient.runIteration(now);
    ANET_LOG(SPAM, "DONE !");
    CPPUNIT_ASSERT_EQUAL(1, handler.requestCount);
    CPPUNIT_ASSERT_EQUAL(1, handler.responseCount);    
    conn->subRef();
    listener->subRef();
    ANET_LOG(DEBUG, "End testHTTPHandler()");
}

void ConnectionTF::testHTTPTimeoutBeforeSend() {
    ANET_LOG(DEBUG, "Begin testHTTPTimeoutBeforeSend()");
    Transport tranServer;
    Transport tranClient;
    int64_t now;
    HTTPPacketFactory factory;
    HTTPStreamer httpStreamer(&factory);
    HTTPHandler handler;    
    char sepc[] = "tcp:localhost:12349";
    IOComponent *listener = tranServer.listen(sepc, &httpStreamer, &handler);
    tranServer.runIteration(now);
    Connection *conn = tranClient.connect(sepc, &httpStreamer);
    tranServer.runIteration(now);
    tranClient.runIteration(now);
    CPPUNIT_ASSERT(tranClient._iocListHead);
    CPPUNIT_ASSERT(tranServer._iocListHead);
    HTTPPacket *packet = new HTTPPacket;
    packet->setPacketType(HTTPPacket::PT_REQUEST);
    packet->setMethod(HTTPPacket::HM_GET);
    packet->setVersion(HTTPPacket::HTTP_1_1);
    packet->setURI("/abc");
    void *args = (void *)0x12345678;
    CPPUNIT_ASSERT(conn->postPacket(packet, &handler, args, false));
    tranClient.timeoutIteration(TimeUtil::PRE_MAX);
    int idx = ControlPacket::CMD_TIMEOUT_PACKET;
    CPPUNIT_ASSERT_EQUAL(1, handler.clientControlCount[idx]);
    conn->subRef();
    listener->subRef();
    ANET_LOG(DEBUG, "End testHTTPTimeoutBeforeSend()");
 }

void ConnectionTF::testHTTPTimeoutAfterSend() {
    ANET_LOG(DEBUG, "Begin testHTTPTimeoutAfterSend()");
    Transport tranServer;
    Transport tranClient;
    int64_t now;
    HTTPPacketFactory factory;
    HTTPStreamer httpStreamer(&factory);
    HTTPHandler handler;    
    char sepc[] = "tcp:localhost:12349";
    IOComponent *listener = tranServer.listen(sepc, &httpStreamer, &handler);
    tranServer.runIteration(now);
    Connection *conn = tranClient.connect(sepc, &httpStreamer);
    tranServer.runIteration(now);
    tranClient.runIteration(now);
    CPPUNIT_ASSERT(tranClient._iocListHead);
    CPPUNIT_ASSERT(tranServer._iocListHead);
    HTTPPacket *packet = new HTTPPacket;
    packet->setPacketType(HTTPPacket::PT_REQUEST);
    packet->setMethod(HTTPPacket::HM_GET);
    packet->setVersion(HTTPPacket::HTTP_1_1);
    packet->setURI("/abc");
    void *args = (void *)0x12345678;
    CPPUNIT_ASSERT(conn->postPacket(packet, &handler, args, false));
    tranClient.runIteration(now);
    CPPUNIT_ASSERT_EQUAL((size_t)0, conn->_outputQueue.size());
    tranClient.timeoutIteration(TimeUtil::PRE_MAX);
    int idx = ControlPacket::CMD_TIMEOUT_PACKET;
    CPPUNIT_ASSERT_EQUAL(1, handler.clientControlCount[idx]);
    conn->subRef();
    listener->subRef();
    ANET_LOG(DEBUG, "End testHTTPTimeoutAfterSend()");
 }


void ConnectionTF::testConstructorNoHeader() {
    ANET_LOG(DEBUG, "Begin testConstructorNoHeader()");
    const char *spec = "tcp:localhost:12345";
    IOComponent *ioc = NULL;
    Transport transport;
    HTTPPacketFactory httpPacketFactory;
    HTTPStreamer httpStreamer(&httpPacketFactory);
    HTTPHandler handler;
    Connection *httpclient = transport.connect(spec, &httpStreamer);
    CPPUNIT_ASSERT_EQUAL((size_t)1, httpclient->getQueueLimit());
    StreamingContext *context = dynamic_cast<HTTPStreamingContext*>(httpclient->_context);
    CPPUNIT_ASSERT(context);
    HTTPPacket *request = new HTTPPacket;
    CPPUNIT_ASSERT(request);
    request->setPacketType(HTTPPacket::PT_REQUEST);
    request->setMethod(HTTPPacket::HM_GET);
    request->setVersion(HTTPPacket::HTTP_1_1);
    request->setURI("/abc");
    CPPUNIT_ASSERT(httpclient->postPacket(request, &handler));
    
    HTTPPacket *response = new HTTPPacket;
    CPPUNIT_ASSERT(response);
    response->setPacketType(HTTPPacket::PT_RESPONSE);
    response->setVersion(HTTPPacket::HTTP_1_1);
    response->setStatusCode(200);
    response->setBody("response", 8);
    ioc = httpclient->getIOComponent();
    ioc->lock();
    CPPUNIT_ASSERT(httpclient->handlePacket(response));
    ioc->unlock();    
    CPPUNIT_ASSERT_EQUAL(1, handler.responseCount);
    transport.closeComponents();
    int idx = ControlPacket::CMD_CONNECTION_CLOSED;
    CPPUNIT_ASSERT_EQUAL(1, handler.clientControlCount[idx]);
    httpclient->subRef();
    ANET_LOG(DEBUG, "End testConstructorNoHeader()");
}

void ConnectionTF::testGetPacketPostedCount() {
    ANET_LOG(DEBUG, "Begin test");
    CPPUNIT_ASSERT_EQUAL(0u, _connection->getPacketPostedCount());

    ConnPacket *packet = new ConnPacket;
    _connection->closeHook();
    CPPUNIT_ASSERT(!_connection->postPacket(packet, NULL, NULL));
    CPPUNIT_ASSERT_EQUAL(0u, _connection->getPacketPostedCount());
    
    _connection->_closed = false;
    CPPUNIT_ASSERT(_connection->postPacket(packet, NULL, NULL));
    CPPUNIT_ASSERT_EQUAL(1u, _connection->getPacketPostedCount());

    packet = new ConnPacket();
    CPPUNIT_ASSERT(_connection->postPacket(packet, NULL, NULL));
    CPPUNIT_ASSERT_EQUAL(2u, _connection->getPacketPostedCount());

    CPPUNIT_ASSERT_EQUAL(0u, _connection->getPacketHandledCount());
}

void ConnectionTF::testGetPacketHandledCount() {
    ANET_LOG(DEBUG, "Begin test");
    CPPUNIT_ASSERT_EQUAL(0u, _connection->getPacketHandledCount());

    Channel *channel;
    ChannelPool *channelPool = &_connection->_channelPool;
    DefaultPacket *packet = new ConnPacket;

    packet->setChannelId(2);
    //No handler found.
    CPPUNIT_ASSERT(!_connection->handlePacket(packet));
    CPPUNIT_ASSERT_EQUAL(0u, _connection->getPacketHandledCount());

    //use handler in channel
    packet = new ConnPacket;
    channel = channelPool->allocChannel(0);
    packet->setChannelId(channel->getId());
    channel->setHandler(_handler);

    CPPUNIT_ASSERT(_connection->handlePacket(packet));
    CPPUNIT_ASSERT_EQUAL(1u, _connection->getPacketHandledCount());

    //use default handler
    packet = new ConnPacket;
    _connection->setDefaultPacketHandler(_handler);
    channel = channelPool->allocChannel(0);
    packet->setChannelId(channel->getId());
    channel->setHandler(NULL);
    CPPUNIT_ASSERT(_connection->handlePacket(packet));
    CPPUNIT_ASSERT_EQUAL(2u, _connection->getPacketHandledCount());

    //isServer=true
    _connection->setServer(true);
    packet = new ConnPacket;
    CPPUNIT_ASSERT(_connection->handlePacket(packet));
    CPPUNIT_ASSERT_EQUAL(3u, _connection->getPacketHandledCount());

    CPPUNIT_ASSERT_EQUAL(0u, _connection->getPacketPostedCount());
}

IPacketHandler::HPRetCode 
HTTPHandler::handlePacket(Packet *packet, void *args) {
    if (packet->isRegularPacket()) {
        responseCount ++;
    } else {
        ControlPacket *cmd = (ControlPacket*) packet;
        ANET_LOG(DEBUG, "CMD:|%s| received in client", cmd->what());
        clientControlCount[cmd->getCommand()] ++;
    }
    packet->free();
    return IPacketHandler::FREE_CHANNEL;
}

IPacketHandler::HPRetCode 
HTTPHandler::handlePacket(Connection *connection, Packet *packet) {
    if (packet->isRegularPacket()) {
        requestCount ++;
        HTTPPacket *response = new HTTPPacket;
        response->setPacketType(HTTPPacket::PT_RESPONSE);
        response->setVersion(HTTPPacket::HTTP_1_1);
        response->setStatusCode(200);
        response->setReasonPhrase("OK");
        response->setBody("abcd", 4);
        if (!connection->postPacket(response)) {
            response->free();
        }
    } else {
        ControlPacket *cmd = (ControlPacket*) packet;
        ANET_LOG(DEBUG, "CMD:|%s| received in server", cmd->what());
        serverControlCount[cmd->getCommand()] ++;
    }
    packet->free();
    return IPacketHandler::FREE_CHANNEL;
}

}
