/**
 * The "helloworld_s" is a dummy anet server application just to show how
 * simple it can be to build a server application using anet libarary. It
 * listens to 0.0.0.0:5555 to accept incoming connections from any number
 * of client. Once a connection between client and helloworld_s establised,
 * the client can send request to helloworld_s through anet libarary, and
 * the helloworld_s will get that request and reverse the request and send
 * it back to the client.
 */
#include <anet/anet.h>
#include <algorithm>
#include <anet/log.h>
#include <signal.h>
#include <iostream>
using namespace anet;
class HelloWorldServerAdapter : public IServerAdapter {
public:
    IPacketHandler::HPRetCode 
    handlePacket(Connection *connection, Packet *packet) {
        if (packet->isRegularPacket()) {//handle request
            DefaultPacket *request = dynamic_cast<DefaultPacket*>(packet);
            size_t bodyLength = 0;
            char *requestBody = request->getBody(bodyLength);
            std::reverse(requestBody, requestBody + bodyLength);
            if (!connection->postPacket(request)) {
                ANET_LOG(ERROR,"Faild to send reply packet!");
                request->free();//free packet if failed to post it
            }
        } else {//control command received
            ControlPacket *cmd = dynamic_cast<ControlPacket*>(packet);
            ANET_LOG(WARN, "Control Packet (%s) received!", cmd->what());
            packet->free(); //free packet if finished
        }
        return IPacketHandler::FREE_CHANNEL;
    }
};

Transport gTransport;
void singalHandler(int sig) {
//    ANET_LOG(INFO,"Singal (%d) Caught!", sig);
    std::cerr << "Signal(" << sig << ") received!" << std::endl;
    gTransport.stopRun();//set stop flag in gTransport
}

int main() {
    Logger::logSetup();
    Logger::setLogLevel("DEBUG");
    const char *spec = "tcp:0.0.0.0:5555";
    DefaultPacketFactory factory;
    DefaultPacketStreamer streamer(&factory);
    HelloWorldServerAdapter server;
    IOComponent *listener = gTransport.listen(spec, &streamer, &server, 3000);
    if (NULL == listener) { exit(1); }
    signal(SIGTERM, singalHandler);
    signal(SIGINT, singalHandler);
    ANET_LOG(INFO,"listening to %s", spec);
    gTransport.run(); // employ main thread as anet driver
    gTransport.closeComponents(); //close all components in transport;
    listener->subRef();
    ANET_LOG(INFO,"helloworld_s is about to exit!", spec); 
}
