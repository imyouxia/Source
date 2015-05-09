#include "transporttf.h"
#include <anet/tcpcomponent.h>
#include <anet/defaultpacketfactory.h>
#include <anet/log.h>

namespace anet {
CPPUNIT_TEST_SUITE_REGISTRATION(TransportTF);

TransportTF::TransportTF() {
    CPPUNIT_ASSERT(_factory = new DefaultPacketFactory());
    CPPUNIT_ASSERT(_streamer = new DefaultPacketStreamer(_factory));
    CPPUNIT_ASSERT(_adapter = new EchoServerAdapter());
    CPPUNIT_ASSERT(_handler = new EchoPacketHandler());
}

TransportTF::~TransportTF() {
    delete _handler;
    delete _adapter;
    delete _streamer;
    delete _factory;
}

void TransportTF::testListen() {
    Transport tran;
    IOComponent *ioc = NULL;
    const char *spec = "tcp:localhost:12333";
    CPPUNIT_ASSERT(!tran.listen(NULL, _streamer, _adapter));
    CPPUNIT_ASSERT(!tran.listen("wrongaddress", _streamer, _adapter));
    CPPUNIT_ASSERT(!tran.listen("tcp:wronghost:1234", _streamer, _adapter));
    CPPUNIT_ASSERT(!tran.listen("udp:localhost:12333", _streamer, _adapter));
    CPPUNIT_ASSERT(!tran.listen(spec, _streamer,NULL));
    CPPUNIT_ASSERT(!tran.listen(spec, NULL, _adapter));
    CPPUNIT_ASSERT(ioc = tran.listen(spec, _streamer, _adapter));
    CPPUNIT_ASSERT(ioc->getRef() >= 1);
    CPPUNIT_ASSERT(!ioc->isClosed());
    ioc->close();
    ioc->subRef();
}
    
void TransportTF::testConnect() {
    Transport tran;
    Connection *conn1 = NULL;
    Connection *conn2 = NULL;
    const char *spec = "tcp:localhost:12345";
    CPPUNIT_ASSERT(!tran.connect(NULL, _streamer, false));    //wrong spec
    CPPUNIT_ASSERT(!tran.connect("aa:bb", _streamer, true)); //wrong spec
    CPPUNIT_ASSERT(!tran.connect("nottcp:loclahost:12345", _streamer, false));
    CPPUNIT_ASSERT(!tran.connect("udp:localhost:12345", _streamer, true));
    CPPUNIT_ASSERT(!tran.connect("tcp:256.256.256.256:999", _streamer, false));
    CPPUNIT_ASSERT(!tran.connect("tcp:256.256.256.256:12345", _streamer, false));
    CPPUNIT_ASSERT(!tran.connect(spec, NULL, false));
    CPPUNIT_ASSERT(conn1 = tran.connect(spec, _streamer, false));
    CPPUNIT_ASSERT(!conn1->isClosed());
    CPPUNIT_ASSERT(conn2 = tran.connect(spec, _streamer, true));
    CPPUNIT_ASSERT(!conn2->isClosed());
    CPPUNIT_ASSERT(conn1->getRef() >= 1);
    CPPUNIT_ASSERT(conn2->getRef() >= 1);
    conn1->subRef();
    conn2->subRef();
}
    
void TransportTF::testAddComponent()
{
    Transport *transport = NULL;
    Socket *socket = NULL;
    TCPComponent *ioc = NULL;
    CPPUNIT_ASSERT(transport = new Transport);
    CPPUNIT_ASSERT(socket = new Socket);
    ioc = new TCPComponent(transport, socket);
    CPPUNIT_ASSERT(ioc);
    ioc->createConnection(NULL, NULL);

    CPPUNIT_ASSERT_EQUAL(1,ioc->getRef());
    transport->removeComponent(ioc);        
    CPPUNIT_ASSERT_EQUAL(1,ioc->getRef());
    transport->addComponent(ioc);
    CPPUNIT_ASSERT_EQUAL(2, ioc->getRef());
    transport->addComponent(ioc);
    CPPUNIT_ASSERT_EQUAL(2,ioc->getRef());
    transport->removeComponent(ioc);        
    CPPUNIT_ASSERT_EQUAL(1,ioc->getRef());
    transport->removeComponent(ioc);        
    CPPUNIT_ASSERT_EQUAL(1,ioc->getRef());
    ioc->subRef();
    delete transport;
}

void TransportTF::testEventIteration() {
    Transport *tranServer = NULL;
    Transport *tranClient = NULL;
    IOComponent *listener = NULL;
    Connection *connection = NULL;
    const char *spec = "tcp:localhost:6666";
    int64_t now = TimeUtil::getTime();
    ANET_LOG(SPAM, "Begin testEventIteration()");
    CPPUNIT_ASSERT(tranServer  = new Transport);
    CPPUNIT_ASSERT(tranClient  = new Transport);

    //int refcount;

    CPPUNIT_ASSERT(listener = tranServer->listen(spec, _streamer, _adapter));
    ANET_LOG(SPAM, "After tranServer->listen(spec, _streamer, _adapter)");
    
    //add listener to socketEvent
    tranServer->eventIteration(now);
    ANET_LOG(SPAM,"After tranServer->eventIteration(now)");
    CPPUNIT_ASSERT_EQUAL(0, (int)_adapter->_count.counter);

    //require setting up connection
    CPPUNIT_ASSERT(connection = tranClient->connect(spec, _streamer, false));
    ANET_LOG(SPAM,"After tranClient->connect(spec, _streamer, false)");

    //add connection to socket event
    tranClient->eventIteration(now);
    ANET_LOG(SPAM,"After tranClient->eventIteration(now)");

    //listener accept a connection
    tranServer->eventIteration(now);
    ANET_LOG(SPAM,"After tranServer->eventIteration(now)");
    
    Packet * packet = new EchoPacket();
    //post a packet to output queue
    CPPUNIT_ASSERT(connection->postPacket(packet, _handler, NULL, false));
    ANET_LOG(SPAM,"After connection->postPacket(packet, _handler, NULL, false)");
    
    //send request via tcp
    tranClient->eventIteration(now);
    ANET_LOG(SPAM,"After tranClient->eventIteration(now)");

    //accepted connection handle read
    tranServer->eventIteration(now);
    ANET_LOG(SPAM,"After tranServer->eventIteration(now)");
    CPPUNIT_ASSERT_EQUAL(1, (int)_adapter->_count.counter);

    //accepted send reply
    tranServer->eventIteration(now);
    ANET_LOG(SPAM,"After tranServer->eventIteration(now)");

    //client receive reply
    tranClient->eventIteration(now);
    ANET_LOG(SPAM,"After tranClient->eventIteration(now)");
    CPPUNIT_ASSERT_EQUAL(1, (int)_handler->_count.counter);

    //close server (both listener and accepted connection
    delete tranServer;
    ANET_LOG(SPAM,"After delete tranServer");
    CPPUNIT_ASSERT_EQUAL(1, listener->getRef());
    CPPUNIT_ASSERT(listener->isClosed());
    listener->subRef();

    //client close connection
    tranClient->eventIteration(now);
    ANET_LOG(SPAM,"After tranClient->eventIteration(now)");
    CPPUNIT_ASSERT(connection->isClosed());

    delete tranClient;
    ANET_LOG(SPAM,"After delete tranClient");
    CPPUNIT_ASSERT_EQUAL(1, connection->getRef());
    connection->subRef();
}

void TransportTF::testTimeoutIteration() {
    Transport *tranServer = NULL;
    Transport *tranClient = NULL;
    IOComponent *listener = NULL;
    Connection *connection = NULL;
    const char *spec = "tcp:localhost:6666";
    int64_t now = TimeUtil::getTime();
    int64_t now2;
    ANET_LOG(SPAM, "Begin testTimeoutIteration()");
    CPPUNIT_ASSERT(tranServer  = new Transport);
    CPPUNIT_ASSERT(tranClient  = new Transport);
    CPPUNIT_ASSERT(listener = tranServer->listen(spec, _streamer, _adapter));
    CPPUNIT_ASSERT(connection = tranClient->connect(spec, _streamer, false));
    tranServer->eventIteration(now);
    ANET_LOG(DEBUG," after tranServer->eventIteration(%ld)", now);
    tranServer->_commands[1].ioc->updateUseTime(now);//hack!!!

    tranClient->eventIteration(now2);//now we have setup a connection

    tranServer->timeoutIteration(now + MAX_IDLE_TIME - 1);
    ANET_LOG(SPAM,"After tranServer->timeoutIteration(now + MAX_IDLE_TIME - 1)");
    tranClient->eventIteration(now2);
    ANET_LOG(SPAM,"After tranClient->eventIteration(now2)");
    CPPUNIT_ASSERT(!connection->isClosed());
    tranServer->timeoutIteration(now + MAX_IDLE_TIME);
    ANET_LOG(SPAM,"After tranServer->timeoutIteration(now + MAX_IDLE_TIME)");
    tranClient->eventIteration(now2);
    CPPUNIT_ASSERT(!connection->isClosed());
    tranServer->timeoutIteration(now + MAX_IDLE_TIME + 1);
    ANET_LOG(SPAM,"After tranServer->timeoutIteration(now + MAX_IDLE_TIME + 1)");
    tranClient->eventIteration(now2);
    ANET_LOG(SPAM,"After tranServer->eventIteration()");
    CPPUNIT_ASSERT(connection->isClosed());
    CPPUNIT_ASSERT_EQUAL(2, connection->getRef());
    connection->subRef();
    ANET_LOG(SPAM,"After connection->subRef();");
    
    delete tranClient;
    delete tranServer;
    listener->subRef();
}

void TransportTF::testClosedByUser() {
    Transport *tranServer = NULL;
    Transport *tranClient = NULL;
    IOComponent *listener = NULL;
    Connection *connection = NULL;
    const char *spec = "tcp:localhost:6666";
    int64_t now = TimeUtil::getTime();
    ANET_LOG(SPAM, "Begin testTimeoutIteration()");
    CPPUNIT_ASSERT(tranServer  = new Transport);
    CPPUNIT_ASSERT(tranClient  = new Transport);
    CPPUNIT_ASSERT(listener = tranServer->listen(spec, _streamer, _adapter));
    CPPUNIT_ASSERT(connection = tranClient->connect(spec, _streamer, false));

    CPPUNIT_ASSERT_EQUAL(3, connection->getRef());
    connection->close();
    CPPUNIT_ASSERT(connection->isClosed());
    CPPUNIT_ASSERT_EQUAL(4, connection->getRef());
    tranClient->eventIteration(now);  
    CPPUNIT_ASSERT_EQUAL(2, connection->getRef());
    tranClient->timeoutIteration(now);
    CPPUNIT_ASSERT_EQUAL(2, connection->getRef());
    connection->subRef();
    ANET_LOG(SPAM,"After connection->subRef()");
    tranClient->timeoutIteration(now);
    ANET_LOG(SPAM,"After tranClient->timeoutIteration(now)");
    delete tranClient;
    delete tranServer;
    listener->subRef();
}

void TransportTF::testAutoReconnect() {
    Transport *tranServer = NULL;
    Transport *tranClient = NULL;
    IOComponent *listener = NULL;
    Connection *connection = NULL;
    const char *spec = "tcp:localhost:6666";
    int64_t now = TimeUtil::getTime();
    ANET_LOG(SPAM, "Begin testTimeoutIteration()");
    CPPUNIT_ASSERT(tranServer  = new Transport);
    CPPUNIT_ASSERT(tranClient  = new Transport);
    CPPUNIT_ASSERT(listener = tranServer->listen(spec, _streamer, _adapter));
    CPPUNIT_ASSERT(connection = tranClient->connect(spec, _streamer, true));
    tranServer->eventIteration(now);
    tranClient->eventIteration(now);//now we have setup a connection

    CPPUNIT_ASSERT(!connection->isClosed());

    //shut down server
    delete  tranServer;
    listener->subRef();
    tranClient->eventIteration(now);
    CPPUNIT_ASSERT(!connection->isClosed());    
    for (int i = 2; i < 2 * MAX_RECONNECTING_TIMES; i++) {
        now += (CONNECTING_TIMEOUT + 1) ;//add two second
        tranClient->timeoutIteration(now);
        CPPUNIT_ASSERT(!connection->isClosed());    
    }
    
    //will not do the last reconnect
    tranClient->timeoutIteration(now + RECONNECTING_INTERVAL);
    CPPUNIT_ASSERT(!connection->isClosed());    

    //do the last reconnect
    now += (RECONNECTING_INTERVAL + 1);
    tranClient->timeoutIteration(now);
    CPPUNIT_ASSERT(!connection->isClosed());    

    //will not timeout
    now += (CONNECTING_TIMEOUT + 1 - RECONNECTING_INTERVAL);
    tranClient->timeoutIteration(now);
    CPPUNIT_ASSERT(!connection->isClosed());    

    //last timeout
    now += RECONNECTING_INTERVAL;
    tranClient->timeoutIteration(now);
    CPPUNIT_ASSERT(connection->isClosed());    

    connection->subRef();
    delete tranClient;
}

void TransportTF::testConstructor() {
    Transport transport;
    CPPUNIT_ASSERT(!transport._stop);
    CPPUNIT_ASSERT(!transport._started);
    CPPUNIT_ASSERT(!transport._iocListHead);
    CPPUNIT_ASSERT(!transport._iocListTail);
    CPPUNIT_ASSERT(!transport._promotePriority);
    CPPUNIT_ASSERT_EQUAL((int64_t)0, transport._nextCheckTime);
    transport.stopRun();
    CPPUNIT_ASSERT(transport._stop);
}

void TransportTF::testRun() {
    Transport transport;
    int64_t now = TimeUtil::getTime();
    transport.timeoutIteration(now);
    CPPUNIT_ASSERT_EQUAL(now + 100000, transport._nextCheckTime);
    int64_t newtime = 0;
    int64_t lastCheckTime = TimeUtil::getTime() - 101000;
    transport._nextCheckTime = lastCheckTime;

    transport.runIteration(newtime);
    CPPUNIT_ASSERT(newtime >= now);
    CPPUNIT_ASSERT_EQUAL(newtime + 100000, transport._nextCheckTime);

    lastCheckTime = newtime + 300000;
    transport._nextCheckTime = lastCheckTime;
    transport.runIteration(newtime);
    CPPUNIT_ASSERT_EQUAL(lastCheckTime, transport._nextCheckTime);
}
}
