#include "tcpcomponenttf.h"
#include <anet/tcpcomponent.h>
#include "testadapter.h"
#include <anet/defaultpacketstreamer.h>
#include <anet/defaultpacketfactory.h>
#include <memory>
#include <anet/serversocket.h>
#include <anet/socket.h>
#include <anet/epollsocketevent.h>
#include <unistd.h>

using namespace std;

namespace anet
{
CPPUNIT_TEST_SUITE_REGISTRATION(TCPCOMPONENTTF);

TCPCOMPONENTTF::TCPCOMPONENTTF() {
    _factory = new DefaultPacketFactory;
    _streamer = new DefaultPacketStreamer(_factory);
}

TCPCOMPONENTTF::~TCPCOMPONENTTF() {
    delete _streamer;
    delete _factory;
}

void TCPCOMPONENTTF::setUp()
{
    ;
}

void TCPCOMPONENTTF::tearDown()
{
    ;
}

int TCPCOMPONENTTF::getListCountFromHead(IOComponent *ioc)
{
    int count = 0;
    while (ioc) {
        ioc = ioc->_next;
        count ++;
    }
    return count;
}    
void TCPCOMPONENTTF::testSocketConnectAndClose()
{
    Transport transport;
    Socket *socket = new Socket();
    socket->checkSocketHandle();
    TCPComponent tcpComponent(&transport, socket);
//    tcpComponent.createConnection(NULL, NULL);
    // connect failed
    CPPUNIT_ASSERT(!tcpComponent.socketConnect());
    CPPUNIT_ASSERT(socket->setAddress("localhost", 11125));
    CPPUNIT_ASSERT(tcpComponent.socketConnect());
    CPPUNIT_ASSERT_EQUAL(true, tcpComponent.isConnectState());

    tcpComponent.createConnection(_streamer, NULL);

    tcpComponent.close();
    CPPUNIT_ASSERT_EQUAL(false, tcpComponent.isConnectState());
    //the socket is delete in transport.cpp
}

void TCPCOMPONENTTF::testHandleErrorEventConnecting() {//added for ticket #23
    Transport transport;
    Socket *socket = new Socket();
    CPPUNIT_ASSERT(socket);
    CPPUNIT_ASSERT(socket->setAddress("localhost", 11126));
    TCPComponent tcpComponent(&transport, socket);
    //tcpComponent.createConnection(NULL, NULL);
    tcpComponent.createConnection(_streamer, NULL);

    //ANET_CONNECTING ==Error&&reconn==> ANET_TO_BE_CONNECTING
    tcpComponent.setState(IOComponent::ANET_CONNECTING);
    tcpComponent.setAutoReconn(true);
    CPPUNIT_ASSERT(!tcpComponent.handleErrorEvent());
    CPPUNIT_ASSERT_EQUAL(IOComponent::ANET_TO_BE_CONNECTING,
                         tcpComponent.getState());

    //ANET_CONNECTING ==Error&&!reconn==> ANET_CLOSING
    tcpComponent.setState(IOComponent::ANET_CONNECTING);
    tcpComponent.setAutoReconn(false);
    CPPUNIT_ASSERT(!tcpComponent.handleErrorEvent());
    CPPUNIT_ASSERT_EQUAL(IOComponent::ANET_CLOSING, tcpComponent.getState());
}

void TCPCOMPONENTTF::testHandleErrorEventConnected() {//added for ticket #23
    Transport transport;
    Socket *socket = new Socket();
    CPPUNIT_ASSERT(socket);
    CPPUNIT_ASSERT(socket->setAddress("localhost", 11126));
    TCPComponent tcpComponent(&transport, socket);
    //tcpComponent.createConnection(NULL, NULL);
    tcpComponent.createConnection(_streamer, NULL);

    //ANET_CONNECTED ==Error&&reconn==> ANET_TO_BE_CONNECTING
    tcpComponent.setState(IOComponent::ANET_CONNECTED);
    tcpComponent.setAutoReconn(true);
    CPPUNIT_ASSERT(!tcpComponent.handleErrorEvent());
    CPPUNIT_ASSERT_EQUAL(IOComponent::ANET_TO_BE_CONNECTING,
                         tcpComponent.getState());

    //ANET_CONNECTED ==Error&&!reconn==> ANET_CLOSING
    tcpComponent.setState(IOComponent::ANET_CONNECTED);
    tcpComponent.setAutoReconn(false);
    CPPUNIT_ASSERT(!tcpComponent.handleErrorEvent());
    CPPUNIT_ASSERT_EQUAL(IOComponent::ANET_CLOSING, tcpComponent.getState());
}


void TCPCOMPONENTTF::testHandleWriteEvent() {
    Transport transport;
    Socket *socket = new Socket();
    socket->checkSocketHandle();
    TCPComponent tcpComponent(&transport, socket);
    //tcpComponent.createConnection(NULL, NULL);
    tcpComponent.createConnection(_streamer, NULL);

    tcpComponent._state = IOComponent::ANET_CONNECTING;
    //error == 0 and connect state is CONNECTING
    CPPUNIT_ASSERT(tcpComponent.handleWriteEvent());
    tcpComponent.close();
    //the socket is closed
    CPPUNIT_ASSERT(!tcpComponent.handleWriteEvent());
    
    //correct connect
    tcpComponent._state = IOComponent::ANET_CONNECTED;
    CPPUNIT_ASSERT(tcpComponent.handleWriteEvent());
}

void TCPCOMPONENTTF::testHandleWriteEventReconnect() {
    Transport transport;
    Socket *socket = new Socket();
    socket->checkSocketHandle();
    TCPComponent tcpComponent(&transport, socket);
    //tcpComponent.createConnection(NULL, NULL);
    Connection *conn = 
        tcpComponent.createConnection(_streamer, NULL);

    tcpComponent._state = IOComponent::ANET_CONNECTING;
    tcpComponent._autoReconn = 1;
    StreamingContext *context = conn->getContext();
    context->setEndOfFile(true);

    //error == 0 and connect state is CONNECTING
    CPPUNIT_ASSERT(tcpComponent.handleWriteEvent());
    CPPUNIT_ASSERT_EQUAL(IOComponent::ANET_CONNECTED, 
                         tcpComponent._state);
    //check whether StreamingContext was reset()
    CPPUNIT_ASSERT_EQUAL(false, context->isEndOfFile());
    CPPUNIT_ASSERT_EQUAL(MAX_RECONNECTING_TIMES, 
                         tcpComponent._autoReconn);


    tcpComponent._state = IOComponent::ANET_CONNECTING;
    tcpComponent._autoReconn = 0;
    CPPUNIT_ASSERT(tcpComponent.handleWriteEvent());
    CPPUNIT_ASSERT_EQUAL(0, tcpComponent._autoReconn);
}

void TCPCOMPONENTTF::testHandleReadEvent()
{
    Transport transport;
    Socket *socket = new Socket();
    socket->checkSocketHandle();
    TCPComponent tcpComponent(&transport, socket);
    //tcpComponent.createConnection(NULL, NULL);
    tcpComponent.createConnection(_streamer, NULL);

    tcpComponent.close();
    //the socket is closed
    CPPUNIT_ASSERT(!tcpComponent.handleReadEvent());
    
    //correct connect
    tcpComponent._state = IOComponent::ANET_CONNECTED;
    CPPUNIT_ASSERT(!tcpComponent.handleReadEvent());
}

void TCPCOMPONENTTF::testInit() {// added for ticket #23
    //There are too many coupling in init()!!!
    Transport tran;
    Socket *socket = new Socket();
//    Connection *fakeConnection = (Connection*)0xfabcd234;
    TCPComponent tcpComponent(&tran, socket);
    CPPUNIT_ASSERT(!tcpComponent.init(false));

    //Noboday listening
    CPPUNIT_ASSERT(socket->setAddress("localhost",12345));
    int64_t before = TimeUtil::getTime();
    CPPUNIT_ASSERT(tcpComponent.init(false));
    int64_t after = TimeUtil::getTime();

    CPPUNIT_ASSERT(tcpComponent._startConnectTime >= before);
    CPPUNIT_ASSERT(tcpComponent._startConnectTime <= after);
}


void TCPCOMPONENTTF::testCreateConnection() {
    Transport tran;
    Socket *socket = new Socket();
    TCPComponent tcpComponent(&tran, socket);
    IServerAdapter *fakeAdapter = (IServerAdapter*)0xfa43212;
    CPPUNIT_ASSERT(socket->setAddress("localhost",12345));
    CPPUNIT_ASSERT(!tcpComponent.createConnection(NULL, NULL));
    CPPUNIT_ASSERT(tcpComponent.init(true));
    CPPUNIT_ASSERT(!tcpComponent.createConnection(_streamer,NULL));
    CPPUNIT_ASSERT(!tcpComponent.createConnection(_streamer, NULL));
    Connection *conn = tcpComponent.createConnection(_streamer, fakeAdapter);
    CPPUNIT_ASSERT(conn);
    CPPUNIT_ASSERT(conn->isServer());
}

    
void TCPCOMPONENTTF::testCheckTimeout()
{
    Transport tran;
    Socket *socket = new Socket();
    TCPComponent tcpComponent(&tran, socket);
    tcpComponent.createConnection(_streamer, NULL);

    int64_t now = TimeUtil::getTime();

    //_state == ANET_CONNECTING
    tcpComponent._state = IOComponent::ANET_CONNECTING;
    tcpComponent._autoReconn = 0;
    tcpComponent._startConnectTime = now - (int64_t)2000001;
    tcpComponent.checkTimeout(now);    
    CPPUNIT_ASSERT_EQUAL(IOComponent::ANET_CLOSING, tcpComponent._state);
    CPPUNIT_ASSERT_EQUAL(-1, socket->getSocketHandle());

    tcpComponent._state = IOComponent::ANET_CONNECTING;
    tcpComponent._autoReconn = 1;
    tcpComponent._startConnectTime = now - (int64_t)2000001;
    tcpComponent.checkTimeout(now);    
    CPPUNIT_ASSERT_EQUAL(IOComponent::ANET_TO_BE_CONNECTING, tcpComponent._state);
    CPPUNIT_ASSERT_EQUAL(-1, socket->getSocketHandle());

    //_state == ANET_CONNECTED
    tcpComponent._state = IOComponent::ANET_CONNECTED;
    tcpComponent._autoReconn = 0;
    tcpComponent._lastUseTime = now - 900000001;
    tcpComponent._isServer = true;
    tcpComponent.checkTimeout(now);
    CPPUNIT_ASSERT_EQUAL(IOComponent::ANET_CLOSING, tcpComponent._state);
    CPPUNIT_ASSERT_EQUAL(-1, socket->getSocketHandle());

    tcpComponent._state = IOComponent::ANET_CONNECTED;
    tcpComponent._autoReconn = 1;
    tcpComponent._lastUseTime =now - 900000001;
    tcpComponent._isServer = true;
    tcpComponent.checkTimeout(now);
    CPPUNIT_ASSERT_EQUAL(IOComponent::ANET_CLOSING, tcpComponent._state);
    CPPUNIT_ASSERT_EQUAL(-1, socket->getSocketHandle());

    tcpComponent._state = IOComponent::ANET_TO_BE_CONNECTING;
    tcpComponent._autoReconn = 1;
    tcpComponent._startConnectTime = now - RECONNECTING_INTERVAL - 1;
    tcpComponent._isServer = false;
    tcpComponent.checkTimeout(now);
    CPPUNIT_ASSERT_EQUAL(IOComponent::ANET_CLOSING, tcpComponent._state);
    CPPUNIT_ASSERT_EQUAL(-1, socket->getSocketHandle());
}
void TCPCOMPONENTTF::testIdleTime()
{
    char sepc[] = "tcp:localhost:1388";
    int64_t now = TimeUtil::getTime();
    Transport *tranServer = new Transport;
    Transport *tranClient = new Transport;
    ConnPacketFactory factory;
    DefaultPacketStreamer streamer(&factory);
    ConnServerAdapter adapter;

    ANET_LOG(SPAM,"Begin testIdleTime()");    
    //add listener to tranServer
    IOComponent *listener = tranServer->listen(sepc, &streamer, &adapter);
    ANET_LOG(SPAM,"After tranServer->listen()");
    tranServer->eventIteration(now);
    ANET_LOG(SPAM,"After tranServer->eventIteration(now)");
    
    //create a connection
    Connection *conn = tranClient->connect(sepc, &streamer, false);
    ANET_LOG(SPAM,"After tranClient->connect(spec, _streamer, false)");
    CPPUNIT_ASSERT(conn);
    tranClient->eventIteration(now);
    ANET_LOG(SPAM,"After tranClient->eventIteration(now)");

    //accept the connection
    tranServer->eventIteration(now);
    ANET_LOG(SPAM,"After tranServer->eventIteration(now)");

    CPPUNIT_ASSERT(tranServer->_iocListHead !=  tranServer->_iocListTail);    
    CPPUNIT_ASSERT_EQUAL(1, getListCountFromHead(tranClient->_iocListHead));
    CPPUNIT_ASSERT_EQUAL(2, getListCountFromHead(tranServer->_iocListHead));
    
    IOComponent *iocClient = tranClient->_iocListTail;
    IOComponent *iocServer = tranServer->_iocListTail;
    
    ANET_LOG(SPAM, "iocClient(%p) before checkTimeout", iocServer);
    iocClient->checkTimeout(now + MAX_IDLE_TIME + 1);
    ANET_LOG(SPAM, "iocClient(%p) after checkTimeout", iocServer);

    tranClient->eventIteration(now);
    tranServer->eventIteration(now);

    CPPUNIT_ASSERT_EQUAL(1, getListCountFromHead(tranClient->_iocListHead));
    CPPUNIT_ASSERT_EQUAL(2, getListCountFromHead(tranServer->_iocListHead));    
    conn->close();
    conn->subRef();

    //server checktimeout
    conn = tranClient->connect(sepc, &streamer, false);
    CPPUNIT_ASSERT(conn);
    tranClient->eventIteration(now);

    //accept the connection
    tranServer->eventIteration(now);

    CPPUNIT_ASSERT_EQUAL(1, getListCountFromHead(tranClient->_iocListHead));
    CPPUNIT_ASSERT_EQUAL(2, getListCountFromHead(tranServer->_iocListHead));
    
    iocClient = tranClient->_iocListTail;
    iocServer = tranServer->_iocListTail;
    
    ANET_LOG(SPAM, "iocServer(%p) before checkTimeout", iocServer);
    iocServer->updateUseTime(now);
    iocServer->checkTimeout(now + MAX_IDLE_TIME + 1);
    ANET_LOG(SPAM, "iocServer(%p) after checkTimeout", iocServer);

    tranClient->eventIteration(now);
    tranServer->eventIteration(now);

    CPPUNIT_ASSERT_EQUAL(1, getListCountFromHead(tranServer->_iocListHead));        
    CPPUNIT_ASSERT_EQUAL(0, getListCountFromHead(tranClient->_iocListHead));

    conn->subRef();
    listener->subRef();
    delete tranClient;
    delete tranServer;

}

void TCPCOMPONENTTF::testCloseSocketNoLock() {
    ANET_LOG(DEBUG,"begin testCloseSocketNoLock()");
    Transport transport;
    IOEvent events[8];
    const char *addr = "127.0.0.1";
    ServerSocket *server = new ServerSocket;
    auto_ptr<ServerSocket> ptr(server);
    Socket *client= new Socket;
    server->setAddress(addr, 4324);
    client->setAddress(addr, 4324);
    server->listen();
    client->connect();
    TCPComponent componet(&transport, client);
    int fd = dup(client->getSocketHandle());
    SocketEvent *se = transport.getSocketEvent();
    se->addEvent(client, false, true);
    CPPUNIT_ASSERT_EQUAL(1, se->getEvents(250, events, 8));
    componet.closeSocketNoLock();
    ANET_LOG(DEBUG,"after componet.closeSocketNoLock()");
    CPPUNIT_ASSERT_EQUAL(0, se->getEvents(250, events, 8));    
    ANET_LOG(DEBUG,"after last assert");
    close(fd);
}

}
