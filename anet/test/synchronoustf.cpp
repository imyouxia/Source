#include "synchronoustf.h"
#include <anet/log.h>
#include <memory>

using namespace std;
namespace anet{
CPPUNIT_TEST_SUITE_REGISTRATION(SynchronousTF);
class SynchronousTestingServer : public IServerAdapter {
public:
    IPacketHandler::HPRetCode handlePacket(Connection *conn, Packet *packet) {
        if (!packet->isRegularPacket()) {
            ControlPacket *cmd = dynamic_cast<ControlPacket*>(packet);
            ANET_LOG(DEBUG, "%s", cmd->what());
            packet->free();
            return IPacketHandler::FREE_CHANNEL;
        }
        DefaultPacket *defaultPacket = dynamic_cast<DefaultPacket*>(packet);
        if (NULL == defaultPacket) {
            ANET_LOG(DEBUG, "Drop unrecognized packet");
            packet->free();
            return IPacketHandler::FREE_CHANNEL;
        }
        string request(defaultPacket->getBody(), defaultPacket->getBodyLen());
        bool hasSend = false;
        if ("drop me" == request) {
            ANET_LOG(DEBUG, "drop request as request.");
        } else if ("close me" == request) {
            ANET_LOG(DEBUG, "Closing Connection as request");
            conn->close();
        } else {
            ANET_LOG(DEBUG, "send request back:|%s|", request.c_str());
            hasSend = conn->postPacket(packet);
        }
        if (!hasSend) {
            ANET_LOG(DEBUG, "free request packet:|%s|", request.c_str());
            packet->free();
        }
        return IPacketHandler::FREE_CHANNEL;
    }
};

SynchronousTF::SynchronousTF() {
    _spec = "tcp:localhost:12345";
    _streamer = new DefaultPacketStreamer(&_factory);
    _server = new SynchronousTestingServer;
}

SynchronousTF::~SynchronousTF() {
    delete _streamer;
    delete _server;
}

void SynchronousTF::setUp() {
    CPPUNIT_ASSERT(_transport = new Transport);
    _transport->start();
    _listener = _transport->listen(_spec.c_str(), _streamer, _server);
    CPPUNIT_ASSERT(_listener);
    _connection = _transport->connect(_spec.c_str(), _streamer);
    _connection->setQueueTimeout(200);
    CPPUNIT_ASSERT(_connection);
    _packet = new DefaultPacket;
    _cmd = NULL;
}

void SynchronousTF::tearDown() {
    _listener->close();
    _listener->subRef();
    _connection->close();
    _connection->subRef();
    _transport->stop();
    _transport->wait();
    if (_packet) {
        _packet->free();
    }
    if (_cmd) {
        _cmd->free();
    }
    delete _transport;
}

void SynchronousTF::testSynchronous() {
    ANET_LOG(DEBUG, "Begin Test");
    string request = "a request";
    _packet->setBody(request.c_str(), request.length());

    Packet *ret = _connection->sendPacket(_packet);
    CPPUNIT_ASSERT(ret);

    _packet = dynamic_cast<DefaultPacket*>(ret);
    CPPUNIT_ASSERT(_packet);

    string reply(_packet->getBody(), _packet->getBodyLen());
    CPPUNIT_ASSERT_EQUAL(request, reply);
}

void SynchronousTF::testSynchronousTimeOut() {
    ANET_LOG(DEBUG, "Begin Test");
    string request = "drop me";
    _packet->setBody(request.c_str(), request.length());

    Packet *ret = _connection->sendPacket(_packet);
    CPPUNIT_ASSERT(ret);
    _packet = NULL;

    _cmd = dynamic_cast<ControlPacket*>(ret);
    CPPUNIT_ASSERT(_cmd);
    CPPUNIT_ASSERT_EQUAL(string("Packet Timeout"), string(_cmd->what()));
}

void SynchronousTF::testSynchronousClosedByPeer() {
    ANET_LOG(DEBUG, "Begin Test");
    string request = "close me";
    _packet->setBody(request.c_str(), request.length());

    Packet *ret = _connection->sendPacket(_packet);
    CPPUNIT_ASSERT(ret);
    _packet = NULL;

    _cmd = dynamic_cast<ControlPacket*>(ret);
    CPPUNIT_ASSERT(_cmd);
    CPPUNIT_ASSERT_EQUAL(string("Connection closing"), string(_cmd->what()));
}

void SynchronousTF::testSynchronousClosed() {
    ANET_LOG(DEBUG, "Begin Test");
    string request = "another request";
    _packet->setBody(request.c_str(), request.length());
    _connection->close();
    Packet *ret = _connection->sendPacket(_packet);
    CPPUNIT_ASSERT(!ret);
}

}//end namespace
