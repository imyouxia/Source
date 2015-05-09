#include "tcpconnectiontf.h"
#include <anet/channel.h>
#include "testadapter.h"
#include "maliciousstreamer.h"
#include <anet/tcpconnection.h>
#include <anet/tcpcomponent.h>


using namespace std;

namespace anet {
    
  CPPUNIT_TEST_SUITE_REGISTRATION(TCPCONNECTIONTF);

  class TCPServerAdapter : public IServerAdapter{
  public:
    TCPServerAdapter():handNum(0) {}
    IPacketHandler::HPRetCode handlePacket(
					   Connection *connection, Packet *packet) {
      handNum ++;
      packet->free();
      return IPacketHandler::KEEP_CHANNEL;
    }
    int reset() {
      handNum = 0;
      return 0;
    }
    //the num of packets that the tcpserveradapter hand
    int handNum;
  };
  
  TCPCONNECTIONTF::TCPCONNECTIONTF() {
    _factory = new ConnPacketFactory;
    CPPUNIT_ASSERT(_factory);
    _streamer = new DefaultPacketStreamer(_factory);
    CPPUNIT_ASSERT(_streamer);
    _handler = new DefaultPacketHandler;
    CPPUNIT_ASSERT(_handler);
  }

  TCPCONNECTIONTF::~TCPCONNECTIONTF() {
    delete _handler;
    delete _streamer;
    delete _factory;
  }
	
  void TCPCONNECTIONTF::setUp() {
    _transport = new Transport;
    CPPUNIT_ASSERT(_transport);
    _server = new ServerSocket;
    CPPUNIT_ASSERT(_server);
    _server->setAddress("localhost", 12345);
    _server->listen();
    Connection *connection = _transport->connect("tcp:localhost:12345", _streamer);
    CPPUNIT_ASSERT(connection);
    _conn = dynamic_cast<TCPConnection*>(connection);
    _accept = _server->accept();
    CPPUNIT_ASSERT(_accept);

  }
  void TCPCONNECTIONTF::tearDown() {
    _conn->subRef();       
    if (NULL != _transport) delete _transport;
    if (NULL != _server) delete _server;
    if (NULL != _accept) delete _accept;
  }
  int TCPCONNECTIONTF::getListCountFromHead(IOComponent *head) {
    if (!head) return 0;
    int num = 0;
    while (head) {
      num ++;
      head = head->_next;
    }
    return num;
  }
  class TCPAddPacket : public Runnable{
  public:
    void run(Thread *thread, void *args) {
      _posted = _connection->postPacket(_packet, _handler, NULL, true);
    }
    DefaultPacketHandler *_handler;
    TCPConnection *_connection;
    ConnPacket *_packet;
    bool _posted;
  }; 
  void TCPCONNECTIONTF::testWriteData() {
    _conn->setServer(false);
    _conn->_queueLimit = 3;


    //_myQueue.Size is zero
    CPPUNIT_ASSERT(_conn->writeData());

    _conn->_defaultPacketHandler = NULL;

    _conn->postPacket(new ConnPacket(11), _handler, NULL);
    _conn->postPacket(new ConnPacket(21), _handler, NULL);
    _conn->postPacket(new ConnPacket(31), NULL, NULL);


    //test lock
    Thread thread;
    TCPAddPacket addPacket;
    addPacket._connection = _conn;
    addPacket._packet = new ConnPacket;
    addPacket._handler = _handler;
    thread.start(&addPacket, _conn);

    usleep(10000);
    CPPUNIT_ASSERT_EQUAL((size_t)3, _conn->_channelPool.getUseListCount());

    CPPUNIT_ASSERT(_conn->writeData()); 
    
    CPPUNIT_ASSERT_EQUAL(0, _conn->_myQueue.size()
                         + _conn->_output.getDataLen() > 0 ? 1 : 0);
    char data[1024];
    CPPUNIT_ASSERT_EQUAL(111, _accept->read(data, 1024));
    
    //test unlock
    thread.join();
    CPPUNIT_ASSERT_EQUAL((size_t)3, _conn->_channelPool.getUseListCount());
    CPPUNIT_ASSERT(addPacket._posted);

    _server->close();
  }

  void TCPCONNECTIONTF::testReadData() {
    ANET_LOG(DEBUG, "BEGIN testReadData()");
    Socket *client = new Socket;
    client->setAddress("localhost", 12345);
    client->connect();
    Socket *accept = _server->accept();
    CPPUNIT_ASSERT(accept);

    //    ConnPacketFactory *factory = new ConnPacketFactory;
    //    DefaultPacketStreamer *streamer = new DefaultPacketStreamer(factory);
    
    //tricket 47
    TCPConnection *tmpConn = new TCPConnection(accept, _streamer, NULL);
    DataBuffer buffer;
    buffer.writeInt32(ANET_PACKET_FLAG + 1);
    buffer.writeInt32(111);
    buffer.writeInt32(222);
    const char *data = "just for test";
    buffer.writeInt32(strlen(data) - 1);
    buffer.writeBytes(data, strlen(data));
    ANET_LOG(DEBUG, "buffer(%p) length:(%d)",&buffer, buffer.getDataLen());
    client->write(buffer.getData(), buffer.getDataLen());
    CPPUNIT_ASSERT(!tmpConn->readData());

    buffer.clear();
    buffer.writeInt32(ANET_PACKET_FLAG);
    buffer.writeInt32(111);
    buffer.writeInt32(222);
    buffer.writeInt32(-1);
    buffer.writeBytes(data, strlen(data));
    client->write(buffer.getData(), buffer.getDataLen());
    CPPUNIT_ASSERT(!tmpConn->readData());

    buffer.clear();
    buffer.writeInt32(ANET_PACKET_FLAG);
    buffer.writeInt32(111);
    buffer.writeInt32(222);
    buffer.writeInt32(strlen(data)/2);
    buffer.writeBytes(data, strlen(data));
    client->write(buffer.getData(), buffer.getDataLen());
    CPPUNIT_ASSERT(!tmpConn->readData());
    delete tmpConn;
    delete accept;
    client->close();
    delete client;

    //    TCPConnection *conn = new TCPConnection(_client, NULL, NULL);
    
    _conn->setServer(false);
    _conn->postPacket(new ConnPacket(11), _handler, NULL);
    _conn->postPacket(new ConnPacket(21), _handler, NULL);
    _conn->postPacket(new ConnPacket(31), _handler, NULL);
    _conn->writeData();

    // flag(0x416e457) chid pcode datalen 
    //DataBuffer *input = &_conn->_input;
    TCPServerAdapter *adapter = new TCPServerAdapter;
    TCPConnection *connAcc = 
      new TCPConnection(_accept, _streamer, adapter);
    connAcc->setServer(true);
    connAcc->_streamer = _streamer;
    connAcc->_iocomponent = 
      new TCPComponent(_transport, _accept);
    //connAcc->_streamer, NULL);
    CPPUNIT_ASSERT(connAcc->readData());
    CPPUNIT_ASSERT_EQUAL(3, adapter->handNum);

    //test error packet
    adapter->reset(); //set the hand packet num to zero
    _conn->postPacket(new ConnPacket(20, 11), _handler, NULL);
    _conn->postPacket(new ConnPacket(20, 30), _handler, NULL);
    CPPUNIT_ASSERT(_conn->writeData());
    CPPUNIT_ASSERT(connAcc->readData());
    CPPUNIT_ASSERT_EQUAL(2, adapter->handNum);

    ANET_LOG(SPAM, "connAcc(%p), ioc (%p)", connAcc, connAcc->_iocomponent);
    delete connAcc->_iocomponent;
    delete connAcc;
    _accept = NULL;
    delete adapter;
    ANET_LOG(DEBUG,"END testReadData()");
  }

  void TCPCONNECTIONTF::testClose()
  {
    TCPServerAdapter adapter;
    Transport *tranServer = new Transport;
    char spec[] = "tcp:localhost:13345";

    tranServer->start();
    IOComponent *listener = tranServer->listen(spec, _streamer, &adapter);
    CPPUNIT_ASSERT(listener);

    
    //start client
    Transport *tranClient = new Transport;
    tranClient->start();
    Connection *conn = tranClient->connect(spec, _streamer, false);
    CPPUNIT_ASSERT(conn);
    CPPUNIT_ASSERT(_conn->postPacket(new ConnPacket(31), _handler, NULL));
    
    // the close() has not implement
    ANET_LOG(SPAM,"Before Calling _conn->close();");
    _conn->close();
    ANET_LOG(SPAM,"After Calling _conn->close();");
    CPPUNIT_ASSERT(_conn->isClosed());
    ANET_LOG(SPAM,"After Assert(_conn->isClosed();");
    tranClient->stop();
    ANET_LOG(SPAM,"After Calling tranClient->stop();");
    tranClient->wait();
    ANET_LOG(SPAM,"After Calling tranClient->wait();");
    tranServer->stop();
    ANET_LOG(SPAM,"After Calling tran_server->stop();");
    tranServer->wait();
    ANET_LOG(SPAM,"After Calling tranClient->wait();");
    delete tranClient;
    delete tranServer;
    listener->subRef();
    conn->subRef();
  }

  void TCPCONNECTIONTF::testLock()
  {
    class WriteData : public Runnable
    {
    public:
      void run(Thread *thread, void *args)
      {
	TCPConnection *conn = static_cast<TCPConnection *>(args);
	conn->writeData();
      }
        
    };


    _conn->setServer(false);
    _conn->_queueLimit = 3;

    Packet *packet = new ConnPacket;
    packet->setChannelId(0);
    _conn->postPacket(packet, _handler, NULL);
    int cid = packet->getChannelId();
    _conn->postPacket(new ConnPacket, NULL, NULL);
    _conn->postPacket(new ConnPacket, NULL, NULL);
    CPPUNIT_ASSERT_EQUAL((size_t)3, _conn->_channelPool.getUseListCount());
    
    TCPAddPacket addPacket;
    Thread thread;
    addPacket._connection = _conn;
    addPacket._packet = new ConnPacket;
    addPacket._handler = _handler;
    thread.start(&addPacket, NULL);

    usleep(100000);
    CPPUNIT_ASSERT_EQUAL((size_t)3, _conn->_channelPool.getUseListCount());
    _conn->writeData();
    thread.join();
    CPPUNIT_ASSERT_EQUAL((size_t)2, _conn->_channelPool.getUseListCount());
    
    _conn->postPacket(new ConnPacket, NULL, NULL);
    CPPUNIT_ASSERT_EQUAL((size_t)3, _conn->_channelPool.getUseListCount());
    
    addPacket._packet = new ConnPacket;
    thread.start(&addPacket, NULL);
    usleep(100000);
    CPPUNIT_ASSERT_EQUAL((size_t)3, _conn->_channelPool.getUseListCount());
    
    ChannelPool &pool = _conn->_channelPool;
    Channel *channel = pool.findChannel(cid);
    uint64_t now  = TimeUtil::getTime();
    channel->setExpireTime(now-1);
    //    _conn->checkTimeout(now);
    _transport->timeoutIteration(now);
    
    thread.join();
    CPPUNIT_ASSERT_EQUAL((size_t)3, _conn->_channelPool.getUseListCount());
  }

  /**
   * test Server and Client memory leak
   * test whether packets are deleted when connection closed
   */
  void TCPCONNECTIONTF::testMemLeak() {
    char spec[] = "tcp:localhost:13147";
    int64_t now = TimeUtil::getTime();
    Transport *tranServer = new Transport;
    Transport *tranClient = new Transport;

    ConnServerAdapter *adapter = new ConnServerAdapter;

    //add listener to tranServer
    IOComponent *listener = tranServer->listen(spec, _streamer, adapter);
    CPPUNIT_ASSERT(listener);
    tranServer->eventIteration(now);
    
    //create a connection
    Connection *conn = tranClient->connect(spec, _streamer, false);
    CPPUNIT_ASSERT(conn);
    tranClient->eventIteration(now);

    //accept the connection
    tranServer->eventIteration(now);

    // client send two packets

    _conn->postPacket(new ConnPacket, _handler, NULL, true);    
    _conn->postPacket(new ConnPacket, _handler, NULL, true);
    tranClient->eventIteration(now);

    //server accept two packets
    tranServer->eventIteration(now);
    IOComponent *ioc = tranServer->_iocListTail;
    Connection *tmpConn = ((TCPComponent *)ioc)->_connection;
    //client close the connection
    _conn->close();
    tranClient->eventIteration(now);
    
    tranServer->eventIteration(now);
    CPPUNIT_ASSERT_EQUAL((size_t)0, tmpConn->_outputQueue.size());

    delete adapter;
    delete tranClient;
    delete tranServer;
    listener->subRef();
    conn->subRef();
  }

  void TCPCONNECTIONTF::testReadMaliciousDataTooLarge() {
    ANET_LOG(DEBUG, "Begin testReadMaliciousDataTooLarge");
    int64_t now = TimeUtil::getTime();
    Transport tranClient;
    Transport tranServer;
    const char sepc[] = "tcp:localhost:12346";
    ConnPacketFactory factory;
    DefaultPacketStreamer defaultStreamer(&factory);
    DefaultPacketHandler defaultHandler;
    TCPServerAdapter adapter;
    MaliciousStreamer maliciousStreamer(&factory);

    IOComponent *listener = 
      tranServer.listen(sepc, &defaultStreamer, &adapter);
    CPPUNIT_ASSERT(listener);
    ANET_LOG(SPAM,"After tranServer->listen()");
    tranServer.eventIteration(now);
    ANET_LOG(SPAM,"After tranServer->eventIteration(now)");

    //create a connection
    Connection *conn = tranClient.connect(sepc, &maliciousStreamer);
    ANET_LOG(SPAM,"After tranClient->connect(spec, _streamer, false)");
    CPPUNIT_ASSERT(conn);
    tranClient.eventIteration(now);
    ANET_LOG(SPAM,"After tranClient->eventIteration(now)");

    //accept the connection
    tranServer.eventIteration(now);
    ANET_LOG(SPAM,"After tranServer->eventIteration(now)");

    conn->postPacket(new ConnPacket(11), &defaultHandler, NULL);
    maliciousStreamer._maliciousLen = 0x70000000;
    ANET_LOG(DEBUG,"before conn->writeData()");
    conn->writeData();
    ANET_LOG(DEBUG,"before connAcc->readData()");
    tranServer.eventIteration(now);
    ANET_LOG(DEBUG,"after connAcc->readData()");
    tranClient.eventIteration(now);
    CPPUNIT_ASSERT(conn->isClosed());
    ANET_LOG(DEBUG,"after tranClient.eventIteration(now)");

    conn->subRef();
    listener->subRef();
    ANET_LOG(DEBUG, "End testReadMaliciousDataTooLarge");
  }

  void TCPCONNECTIONTF::testReadMaliciousDataInfinitLoop() {
    ANET_LOG(DEBUG, "Begin testReadMaliciousDataInfinitLoop");
    int64_t now = TimeUtil::getTime();
    Transport tranClient;
    Transport tranServer;
    const char sepc[] = "tcp:localhost:12346";
    ConnPacketFactory factory;
    DefaultPacketStreamer defaultStreamer(&factory);
    DefaultPacketHandler defaultHandler;
    TCPServerAdapter adapter;
    MaliciousStreamer maliciousStreamer(&factory);

    
    IOComponent *listener = 
      tranServer.listen(sepc, &defaultStreamer, &adapter);
    CPPUNIT_ASSERT(listener);
    ANET_LOG(SPAM,"After tranServer->listen()");
    tranServer.eventIteration(now);
    ANET_LOG(SPAM,"After tranServer->eventIteration(now)");

    //create a connection
    Connection *conn = tranClient.connect(sepc, &maliciousStreamer);
    ANET_LOG(SPAM,"After tranClient->connect(spec, _streamer, false)");
    CPPUNIT_ASSERT(conn);
    tranClient.eventIteration(now);
    ANET_LOG(SPAM,"After tranClient->eventIteration(now)");

    //accept the connection
    tranServer.eventIteration(now);
    ANET_LOG(SPAM,"After tranServer->eventIteration(now)");

    conn->postPacket(new ConnPacket(11), &defaultHandler, NULL);
    maliciousStreamer._maliciousLen = 0x80000000 + 10;
    ANET_LOG(DEBUG,"before conn->writeData()");
    conn->writeData();
    ANET_LOG(DEBUG,"before connAcc->readData()");
    tranServer.eventIteration(now);
    ANET_LOG(DEBUG,"after connAcc->readData()");
    tranClient.eventIteration(now);
    CPPUNIT_ASSERT(conn->isClosed());
    ANET_LOG(DEBUG,"after tranClient.eventIteration(now)");
    conn->subRef();
    listener->subRef();
    ANET_LOG(DEBUG, "End testReadMaliciousDataInfinitLoop");
  }
}
