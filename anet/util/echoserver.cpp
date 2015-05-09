#include <anet/log.h>
#include <anet/anet.h>
#include <anet/stats.h>
#include <signal.h>
#include <iostream>
#include <string>

using namespace anet;

class EchoServer : public IServerAdapter {
public:
    EchoServer(std::string spec) : _spec(spec) {}
    virtual ~EchoServer() {}
    void start();

    void stop() {
        _transport.stopRun();
    }

    IPacketHandler::HPRetCode handlePacket(Connection *, Packet *);
private:
    std::string _spec;
    Transport _transport;
};

void EchoServer::start() {
    DefaultPacketFactory factory;
    DefaultPacketStreamer streamer(&factory);
    
    IOComponent *ioc = _transport.listen(_spec.c_str(), &streamer, this);
    if (ioc == NULL) {
        ANET_LOG(ERROR, "listen error.");
        return;
    }
    ANET_LOG(INFO, "EchoServer(%s) started.", _spec.c_str());
    _transport.run();
    ANET_LOG(INFO, "EchoServer(%s) Quiting...", _spec.c_str());
    ANET_GLOBAL_STAT.log();
    ioc->close();
    ioc->subRef();
}

IPacketHandler::HPRetCode
EchoServer::handlePacket(Connection *connection, Packet *packet) {
    if (! packet->isRegularPacket()) {
        ANET_LOG(ERROR, "=> ControlPacket: %d", 
                 ((ControlPacket*)packet)->getCommand());
        packet->free();
        return IPacketHandler::FREE_CHANNEL;
    }
    connection->postPacket(packet);
    return IPacketHandler::FREE_CHANNEL;
    /**
     * If you want do something expensive. Do it like this:
     *    connection->addRef();
     *    //store connection and packet somewhere
     *    return IPacketHandler::FREE_CHANNEL
     *
     * when you finished your work somewhere. Do like this:
     *    if (connection->isClosed()) {
     *        connection->subRef();
     *    } else {
     *        connection->postPacket(reply);
     *    }
     */
}
    
EchoServer *gEchoServer;
void singalHandler(int sig) {
    std::cerr << "Signal(" << sig << ") received!" << std::endl;
    gEchoServer->stop();
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        printf("%s [tcp|udp]:ip:port [debug level]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int debugLevel = 0;
    if (3 == argc ) {
        debugLevel = atoi(argv[2]);
    }
    Logger::logSetup();
    Logger::setLogLevel(debugLevel);

    signal(SIGTERM, singalHandler);
    signal(SIGINT, singalHandler);
    signal(SIGPIPE, SIG_IGN);

    EchoServer echoServer(argv[1]);
    gEchoServer = &echoServer;
    echoServer.start();
    return 0;
}
