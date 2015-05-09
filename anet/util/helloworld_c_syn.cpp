/**
 * The "helloworld_c_sycn" is a dummy anet client application just to show
 * how simple it can be to build a client application using synchronous
 * interface of anet libarary.
 * It connects to helloworld_s server listening localhost:5555 then  send 8
 * requests. When it get a reply, it will show the request and reply on
 * the console.
 */
#include <anet/anet.h>
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

int main(int argc, char *argv[]) {
    Logger::logSetup();
    Logger::setLogLevel("INFO");
    const char *spec = "tcp:localhost:5555";
    if (argc > 1) {
        spec = argv[1];
    }

    DefaultPacketFactory factory;
    DefaultPacketStreamer streamer(&factory);
    Transport transport;
    transport.start(); //Using anet in multithreads mode
    ANET_LOG(INFO,"Connecting to %s", spec);
    Connection *connection = transport.connect(spec, &streamer);

    for (int idx = 0; idx < 8; idx ++) {
        DefaultPacket *request = new DefaultPacket();
        request->setBody(gRequest[idx].data(), gRequest[idx].length());
        Packet *packet = connection->sendPacket(request);
        if (!packet) {
            ANET_LOG(ERROR, "Failed to send request[%d]: ", 
                     idx, gRequest[idx].c_str());
            request->free();
            break;
        }
        cout << "Request[" << idx << "]: " << gRequest[idx] << endl;
        cout << "  Reply[" << idx << "]: " ;
        if (packet->isRegularPacket()) {//handle request
            DefaultPacket *reply = dynamic_cast<DefaultPacket*>(packet);
            size_t bodyLength = 0;
            const char *replyBody = reply->getBody(bodyLength);
            string replyString(replyBody, bodyLength);
            cout << replyString << endl;
        } else {//control command received
            ControlPacket *cmd = dynamic_cast<ControlPacket*>(packet);
            cout << "Sorry! " << cmd->what() << endl;
        }
    }

    connection->close();//Close this connection.
    connection->subRef();//Do not use this connection any more.
    transport.stop();
    transport.wait();
    ANET_LOG(INFO,"helloworld_c is about to exit!", spec);
}

