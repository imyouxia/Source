/**
 * The "helloworld_c" is a dummy anet client application just to show how
 * simple it can be to build a client application using anet libarary. It
 * connects to helloworld_s server listening localhost:5555 then  send 8
 * requests. When it get a reply, it will show the request and reply on
 * the console.
 */
#include <anet/anet.h>
#include <anet/threadmutex.h>
#include <anet/log.h>
#include <string>
#include <sstream>
#include <iostream>

using namespace std;
using namespace anet;

string gRequest[8] = {
    "Hello World!",
    "Is this amazing?",
    "!gnizama si tI !seY",
    "I like this game!",
    "Is this simple?",
    "!elpmis si sihT",
    "?hsilgne kaeps uoy naC",
    "!lufrednoW sI sihT",
};

class HelloWorldPacketHandler : public IPacketHandler {
public:
    IPacketHandler::HPRetCode
    handlePacket(Packet *packet, void *args) {
        MutexGuard mutexGuard(&_mutex);
        stringstream ss;
        long idx = (long) args;
        ss << "Request[" << idx << "]: " << gRequest[idx] << endl;
        ss << "  Reply[" << idx << "]: " ;
        if (packet->isRegularPacket()) {//handle request
            DefaultPacket *reply = dynamic_cast<DefaultPacket*>(packet);
            size_t bodyLength = 0;
            const char *replyBody = reply->getBody(bodyLength);
            string replyString(replyBody, bodyLength);
            ss << replyString << endl;
        } else {//control command received
            ControlPacket *cmd = dynamic_cast<ControlPacket*>(packet);
            ss << "Sorry! " << cmd->what() << endl;
        }
        _replyCount ++;
        packet->free(); //free packet if finished
        cout << ss.str() << endl;
        return IPacketHandler::FREE_CHANNEL;
    }
    HelloWorldPacketHandler() {
        _replyCount = 0;
        _totalRequest = 8;
     }
    virtual ~HelloWorldPacketHandler() {}
public:
    int _replyCount;
    int _totalRequest;
    ThreadMutex _mutex;
};

int main() {
    Logger::logSetup();
    Logger::setLogLevel("INFO");
    const char *spec = "tcp:localhost:5555";
    DefaultPacketFactory factory;
    DefaultPacketStreamer streamer(&factory);
    HelloWorldPacketHandler handler;
    Transport transport;
    transport.start(); //Using anet in multithreads mode
    ANET_LOG(INFO,"Connecting to %s", spec);
    Connection *connection = transport.connect(spec, &streamer);
    for (int idx = 0; idx < 8; idx ++) {
        DefaultPacket *request = new DefaultPacket();
        request->setBody(gRequest[idx].data(), gRequest[idx].length());
        if (!connection->postPacket(request, &handler, reinterpret_cast<void*>(idx))) {
            ANET_LOG(ERROR, "Failed to send request[%d]: ", 
                     idx, gRequest[idx].c_str());
            request->free();
        }
    }
    while (handler._replyCount < handler._totalRequest) {
        usleep(100000);
    }
    connection->close();//Close this connection.
    connection->subRef();//Do not use this connection any more.
    transport.stop();
    transport.wait();
    ANET_LOG(INFO,"helloworld_c is about to exit!", spec);
}

