#include <queue>
#include <anet/log.h>
#include <anet/transport.h>
#include <anet/thread.h>
#include <anet/iserveradapter.h>
#include <anet/httprequestpacket.h>
#include <anet/httppacketstreamer.h>
#include <anet/httpresponsepacket.h>
#include <signal.h>
#include <iostream>
using namespace anet;
using namespace std;

struct HttpRequest {
    Connection * _connection;
    Packet * _packet;
};

ThreadCond gCondition;
queue<HttpRequest> gRequests;
bool g_stop = false;

DefaultHttpPacketFactory factory;
HttpPacketStreamer streamer(&factory);

/**
 * packetµÄserverAdapter
 */
class HttpServerAdapter : public IServerAdapter
{
public:
    HttpServerAdapter(IPacketFactory *factory) {
        _factory = factory;
    }
    IPacketHandler::HPRetCode handlePacket(Connection *connection, Packet *packet)
    {
        if (! packet->isRegularPacket()) {
            ANET_LOG(ERROR, "=> ControlPacket: %d", 
                     ((ControlPacket*)packet)->getCommand());
            packet->free();
            return IPacketHandler::FREE_CHANNEL;
        }

        gCondition.lock();
        if (gRequests.size() >= 200) {
            const char *msg = "Server is BUSY!!!";
            gCondition.unlock();
            ANET_LOG(WARN,"Too many Request Comming");
            HttpResponsePacket *reply = (HttpResponsePacket *)factory.createPacket(0);
            reply->setStatus(false);
            reply->setBody(msg, strlen(msg));
            if (!connection->postPacket(reply)) {
                ANET_LOG(WARN,"Failed to Send BUSY REPLY");
                reply->free();
            }
            return IPacketHandler::FREE_CHANNEL;
        }
        
        HttpRequest request;
        request._connection = connection;
        request._packet = packet;
        connection->addRef();
        gRequests.push(request);
        if (gRequests.size() == 1) {
            gCondition.signal();
        }
        gCondition.unlock();
        return IPacketHandler::FREE_CHANNEL;
    }    
public:
    IPacketFactory *_factory;
};

class RequestProcessor : public Runnable {
public:
    void run(Thread* thread, void *args) {
        _id = (long)args;
        ANET_LOG(INFO,"thread(%d) started!", _id);
        while (!g_stop) {
            taskIteration();
        }
        ANET_LOG(INFO,"Stopped! thread(%d) exit!", _id);
    }

    void taskIteration() {
        gCondition.lock();
        while(!g_stop && gRequests.size() == 0) {
            ANET_LOG(SPAM, "Thread(%d) waiting ...",_id);
            gCondition.wait();
            ANET_LOG(SPAM, "Thread(%d) waked up", _id);
        }
        if (g_stop) {
            ANET_LOG(INFO,"Thread (%d) Stopped!", _id);
            gCondition.unlock();
            return;
        }
        HttpRequest request = gRequests.front();
        gRequests.pop();
        gCondition.unlock();
        doReply(request);
    }

    void doReply(HttpRequest &request) {
        ANET_LOG(SPAM, "Processing Request");
        HttpRequestPacket *httpPacket = (HttpRequestPacket*) request._packet;
        HttpResponsePacket *reply = (HttpResponsePacket *)factory.createPacket(0);
        reply->setStatus(true);
        reply->setKeepAlive(httpPacket->isKeepAlive());
        if (!httpPacket->isKeepAlive()) {
            request._connection->setWriteFinishClose(true);
        }
        char *query = httpPacket->getQuery();
        assert(query);
        reply->setBody(query, strlen(query));
        ANET_LOG(SPAM,"\nREPLY LEN:%d\nREPLY:%s\nend", strlen(query), query);
        if ( !request._connection->postPacket(reply) ) {
            ANET_LOG(WARN, "Failed to send reply");
            reply->free();
        };

        httpPacket->free();
        request._connection->subRef();
    }
    long _id;
};

/*
 * server ·þÎñÆ÷
 */
class HttpServer {
public:
    HttpServer(char *spec);
    ~HttpServer();
    void start(int thread_num = 10);
    void stop();
private:
    Transport _transport;
    char *_spec;
};

HttpServer::HttpServer(char *spec)
{
    _spec = strdup(spec);
}

HttpServer::~HttpServer()
{
    if (_spec) {
        free(_spec);
    }
}

HttpServerAdapter serverAdapter(&factory);

void HttpServer::start(int num)
{
    Thread *threads = new Thread [num] ;
    RequestProcessor *runnables = new RequestProcessor [num];
    assert(threads);
    assert(runnables);
    for (long i=0; i<num; i++) {
        ANET_LOG(INFO, "Starting thread(%d)", i);
        threads[i].start(runnables + i, (void*)i);
    }
    
    
    IOComponent *ioc = _transport.listen(_spec, &streamer, &serverAdapter, 5000);
    if (ioc == NULL) {
        ANET_LOG(ERROR, "listen error.");
        return;
    }
    _transport.start();
    while(!g_stop) {
        usleep(100000);
    }
    _transport.stop();
    _transport.wait();

    gCondition.lock();
    while (gRequests.size()) {
        HttpRequest &request = gRequests.front();
        HttpRequestPacket *packet = (HttpRequestPacket*)request._packet;
        ANET_LOG(WARN,"Discard request %d", packet->getQuery());
        request._packet->free();
        request._connection->subRef();
        gRequests.pop();
    }
    gCondition.broadcast();
    gCondition.unlock();
    for (long i=0; i<num; i++) {
        ANET_LOG(INFO, "Join thread(%d)",i);
        threads[i].join();
    }

    delete [] threads;
    delete [] runnables;
}

void HttpServer::stop()
{
    _transport.stop();
}

HttpServer *_httpServer;
void singalHandler(int seg)
{
//    ANET_LOG(INFO,"Singal(%d) received", seg);
    std::cerr << "Signal(" << seg << ") received!" << std::endl;
    g_stop = true;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("%s [tcp|udp]:ip:port thread_num debug_level\n", argv[0]);
        return EXIT_FAILURE;
    }
    int debugLevel = 0;
    int num = atoi(argv[2]);
    if ( num < 1 ) {
        ANET_LOG(WARN, "Invalid thread number. Using default: 1");
        num = 1;
    }
    if (4 == argc ) {
        debugLevel =atoi(argv[3]);
    }
    Logger::logSetup();
    Logger::setLogLevel(debugLevel);

    HttpServer httpServer(argv[1]);
    _httpServer = &httpServer;
    signal(SIGINT, singalHandler);
    signal(SIGTERM, singalHandler);
    httpServer.start(num);
    ANET_LOG(INFO, "exit.");
    return EXIT_SUCCESS;
}


