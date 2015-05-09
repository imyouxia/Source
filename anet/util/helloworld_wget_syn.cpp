/**
 * "helloworld_wget_sync" is a tiny tool to fetch web page just like a 
 * subset of wget. 
 * "$ ./helloworld_wget_sync http://www.xxx.com/you/path/page.html"
 * or 
 * "$ ./helloworld_wget_sync http://www.xxx.com:9999/you/path/page.html"
 * or 
 * "$ ./helloworld_wget_sync www.xxx.com:9999/you/path/page.html"
 */

#include <anet/log.h>
#include <anet/anet.h>
using namespace anet;
int main(int argc, char *argv[]) {
    if(2 > argc) {
        printf("Less address!\n");
        return 1;
    }
    Logger::logSetup();
    Logger::setLogLevel(0);  
    char *address = argv[1];
    char host[100] = {0};
    char path[1024*1024] = {0};
    char spec[200] = {0};
    int port = -1;
    sscanf(address, "http://%[-a-zA-Z0-9.]:%d%s", host, &port, path);
    if(0 == host[0] || -1 == port) {
        sscanf(address, "%[-a-zA-Z0-9.]:%d%s", host, &port, path);
    }
    if(0 == host[0] || -1 == port) {
        port = 80;
        sscanf(address, "http://%[-a-zA-Z0-9.]%s", host, path);
    }
    if(0 == host[0] || -1 == port) {
        sscanf(address, "%[-a-zA-Z0-9.]%s", host, path);
    }
    if(0 == host[0] || -1 == port) {
        printf("Wrong address!\n");
        return 1;
    }
    if(0 == path[0]) {
        path[0] = '/';
        path[1] = 0;
    }
    sprintf(spec, "tcp:%s:%d", host, port);
    Transport transport;
    transport.start();
    Connection *connection = NULL;
    HTTPPacketFactory factory;
    HTTPStreamer streamer(&factory);
    connection = transport.connect(spec, &streamer);
    if (NULL == connection) {
        printf("Failed to connect server %s\n", spec);
        exit(1);
    }
    
    HTTPPacket *requst = new HTTPPacket;
    requst->setMethod(HTTPPacket::HM_GET);
    requst->setURI(path);
    requst->addHeader("Accept", "*/*");
    requst->addHeader("Connection", "Keep-Alive");
    requst->addHeader("Host", (const char*)(spec+4));

    Packet *ret = connection->sendPacket(requst);
    HTTPPacket *reply = NULL;
    if (NULL != ret && ret->isRegularPacket() 
        && (reply = dynamic_cast<HTTPPacket*>(ret))) 
    {
        printf("------------reply from '%s' ----------\r\n", address);
        printf("%s %d %s\r\n", reply->getVersion() ? "HTTP/1.1" : "HTTP/1.0",
               reply->getStatusCode(), reply->getReasonPhrase());
        for (HTTPPacket::ConstHeaderIterator it = reply->headerBegin();
             it != reply->headerEnd(); it ++) 
        {
            printf("%s: %s\r\n", it->first, it->second);
        }
        printf("\r\n");
        if (reply->getBody()) {
            fwrite(reply->getBody(), 1, reply->getBodyLen(), stdout);
        }
        printf("\n-----------end of reply-------------------\n");
    } else {
        printf("Fail to get reply from '%s' ----------\r\n", address);
        ControlPacket *cmd = dynamic_cast<ControlPacket*>(ret);
        if (cmd) {
            printf("%s", cmd->what());
        }
    }
    connection->subRef();
    transport.stop();
    transport.wait();
    return 0;
}
