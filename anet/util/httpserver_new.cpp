#include <iostream>
#include <anet/anet.h>
#include <anet/log.h>
#include <signal.h> 
#include <queue>

using namespace anet;
using namespace std;

bool globalStopFlag = false; 
struct HTTPRequestEntry {
    Connection * _connection;
    HTTPPacket * _packet;
    HTTPRequestEntry() : _connection(NULL), _packet(NULL) {}
};

struct RequestQueue {
    ThreadCond _condition;
    queue<HTTPRequestEntry> _queue;
    size_t _queueLimit;
    size_t _waitCount;
    RequestQueue() : _queueLimit(200), _waitCount(0) {}
} globalQueue;

class HTTPServerAdapter : public IServerAdapter {
public:
    IPacketHandler::HPRetCode 
    handlePacket(Connection *connection, Packet *packet) {
        if (packet->isRegularPacket()) {//handle request
            HTTPPacket *request = dynamic_cast<HTTPPacket*>(packet);
            if (NULL == request) { 
                ANET_LOG(WARN, "Invalid HTTPPacket received");
            } else {
                myHandlePacket(connection, request);
            }
        } else {//control command received
            ControlPacket *cmd = dynamic_cast<ControlPacket*>(packet);
            ANET_LOG(WARN, "Control Packet (%s) received!", cmd->what());
            packet->free(); //free packet if finished
        }
        return IPacketHandler::FREE_CHANNEL;
    }//End of handlePacket()

    void myHandlePacket(Connection *connection, HTTPPacket* request) {
        MutexGuard guard(&(globalQueue._condition));
        if (globalQueue._queue.size() >= globalQueue._queueLimit) {
            HTTPPacket *reply = new HTTPPacket;
            assert(reply);
            reply->setPacketType(HTTPPacket::PT_RESPONSE);
            reply->setStatusCode(503);
            reply->setReasonPhrase("Service Unavailable");
            if (!connection->postPacket(reply)) {
                ANET_LOG(ERROR,"Faild to send reply packet!");
                reply->free();
            }
            request->free();
        } else {//add connection, request to globalQueue for later processing
            HTTPRequestEntry entry;
            connection->addRef();//add a reference count for later using
            entry._connection = connection;
            entry._packet = request;
            globalQueue._queue.push(entry);
            if (globalQueue._waitCount > 0) {
                globalQueue._condition.signal();
            }
        }//End of myHandlePacket(Connection *connection, HTTPPacket* request)
    }
};

class RequestProcessor : public Runnable {
public:
    void run(Thread* thread, void *args) {
        _id = (long)args;
        ANET_LOG(INFO,"thread(%d) started!", _id);
        while (!globalStopFlag) {
            taskIteration();
        }
        ANET_LOG(INFO,"Stopped! thread(%d) exit!", _id);
    }

    void taskIteration() {
        globalQueue._condition.lock();
        while(!globalStopFlag && globalQueue._queue.size() == 0) {
            globalQueue._waitCount ++;
            globalQueue._condition.wait();
            globalQueue._waitCount --;
        }
        if (globalStopFlag) {
            ANET_LOG(INFO,"Thread (%d) Stopped!", _id);
            globalQueue._condition.unlock();
            return;
        }
        HTTPRequestEntry entry = globalQueue._queue.front();
        globalQueue._queue.pop();
        globalQueue._condition.unlock();
        doReply(entry);
    }

    void doReply(HTTPRequestEntry &entry) {
        ANET_LOG(SPAM, "Processing Request");
        HTTPPacket *reply = new HTTPPacket();
        reply->setPacketType(HTTPPacket::PT_RESPONSE);
        reply->setStatusCode(200);
        reply->setKeepAlive(entry._packet->isKeepAlive());
        if (!reply->isKeepAlive()) {
            entry._connection->setWriteFinishClose(true);
        }
        const char *uri = entry._packet->getURI();
        assert(uri);
        reply->setBody(uri, strlen(uri));
        ANET_LOG(SPAM,"\nREPLY LEN:%d\nREPLY:%s\nend", strlen(uri), uri);
        if (!entry._connection->postPacket(reply)) {
            ANET_LOG(WARN, "Failed to send reply");
            reply->free();
        };
        entry._packet->free();
        entry._connection->subRef();
    }
    long _id;
};

void singalHandler(int seg)
{
//    ANET_LOG(INFO,"Singal(%d) received", seg);
    std::cerr << "Signal(" << seg << ") received!" << std::endl;
    globalStopFlag = true;
}

void doProcess(const char* spec, unsigned int num);
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("%s [tcp|udp]:ip:port thread_num debug_level\n", argv[0]);
        return -1;
    }
    int num = atoi(argv[2]);
    if ( num < 1 ) {
        ANET_LOG(WARN, "Invalid thread number. Using default: 1");
        num = 1;
    }

    int debugLevel = 0;
    if (4 == argc ) {
        debugLevel =atoi(argv[3]);
    }
    Logger::logSetup();
    Logger::setLogLevel(debugLevel);
    signal(SIGINT, singalHandler);
    signal(SIGTERM, singalHandler);

    doProcess(argv[1], num);
    ANET_LOG(INFO, "exit.");
}

void doProcess(const char *spec, unsigned int num) {
    Transport transport;
    HTTPPacketFactory factory;
    HTTPStreamer streamer(&factory);
    HTTPServerAdapter serverAdapter;

    Thread *threads = new Thread[num];
    RequestProcessor *runnables = new RequestProcessor [num];
    assert(threads);
    assert(runnables);
    for (long i=0; i<num; i++) {
        ANET_LOG(INFO, "Starting thread(%d)", i);
        threads[i].start(runnables + i, (void*)i);
    }

    transport.start(); //using multithreads mode of anet
    IOComponent *ioc = transport.listen(spec, &streamer, &serverAdapter);
    if (ioc == NULL) {
        ANET_LOG(ERROR, "listen (%s) ERROR.", spec);
        return;
    }

    while (!globalStopFlag) {
        usleep(100000);
    }
    transport.stop();
    transport.wait();

    globalQueue._condition.lock();
    while (globalQueue._queue.size()) {
        HTTPRequestEntry entry = globalQueue._queue.front();
        HTTPPacket *packet = (HTTPPacket*)entry._packet;
        ANET_LOG(WARN,"Discard request %s", packet->getURI());
        entry._packet->free();
        entry._connection->subRef();
        globalQueue._queue.pop();
    }
    globalQueue._condition.broadcast();
    globalQueue._condition.unlock();

    for (long i=0; i<num; i++) {
        ANET_LOG(INFO, "Join thread(%d)",i);
        threads[i].join();
    }

    delete [] threads;
    delete [] runnables;
}
