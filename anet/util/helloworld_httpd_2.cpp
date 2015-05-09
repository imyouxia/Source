/**
 * The "helloworld_httpd" is a simple http server application just to show how
 * to build a http server application using anet libarary. 
 */
#include <anet/anet.h>
#include <anet/log.h>
#include <signal.h> 
#include <queue>
#include <sstream>
#include <unistd.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <iostream>
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

const char *root = NULL;
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
        int64_t begin = TimeUtil::getTime();
        m_id = (long)args;
        ANET_LOG(INFO,"thread(%d) started(%p)!", m_id, this);
        while (!globalStopFlag) {
            taskIteration();
        }
        ANET_LOG(INFO,"Stopped! thread(%d) exit!", m_id);
        int64_t end = TimeUtil::getTime();
        m_totalTime = end - begin;
    }

    void taskIteration() {
        int64_t begin = TimeUtil::getTime();
        globalQueue._condition.lock();
        int64_t afterLock = TimeUtil::getTime();
        m_lockTime += afterLock - begin;
        while(!globalStopFlag && globalQueue._queue.size() == 0) {
            globalQueue._waitCount ++;
            globalQueue._condition.wait();
            globalQueue._waitCount --;
        }
        int64_t afterWait = TimeUtil::getTime();
        m_waitTime += afterWait - afterLock;

        if (globalStopFlag) {
            ANET_LOG(DEBUG,"Thread (%d) Stopped!", m_id);
            globalQueue._condition.unlock();
            return;
        }
        HTTPRequestEntry entry = globalQueue._queue.front();
        globalQueue._queue.pop();
        globalQueue._condition.unlock();
        doReply(entry);
        m_taskProcesssed ++;
        int64_t end = TimeUtil::getTime();
        m_runTime += end - afterWait;
    }

    void doReply(HTTPRequestEntry &entry) {
        ANET_LOG(SPAM, "Processing Request");
        HTTPPacket *reply = new HTTPPacket();
        reply->setPacketType(HTTPPacket::PT_RESPONSE);
        const char *uri = entry._packet->getURI();
        assert(uri);
        if (uri[0] != '/') {
            reply->setStatusCode(404);
        } else {
            stringstream ss;
            ss << root << uri;
            if (uri[strlen(uri) - 1] == '/') {
                ss << "index.html";
            }
            fillBody(ss.str().c_str(), reply);
        }
        reply->setKeepAlive(entry._packet->isKeepAlive());
        if (!reply->isKeepAlive()) {
            entry._connection->setWriteFinishClose(true);
        }
        reply->addHeader("Server", "Helloworld_httpd/1.0 ");
        if (!entry._connection->postPacket(reply)) {
            ANET_LOG(WARN, "Failed to send reply");
            reply->free();
        };
        entry._packet->free();
        entry._connection->subRef();
//        for (int i = 0; i < 1024 * 1024 * 5; i ++) {
//            static int j = 2;
//            j *= 2;
//        }
    }

    void fillBody(const string &filename, HTTPPacket *reply) {
        int fd = open(filename.c_str(), O_RDONLY);
        if ( -1 == fd ) {
            reply->setStatusCode(404);
            return;
        }
        bool ok = false;
        struct stat s;
        if (-1 != fstat(fd, &s) && S_ISREG(s.st_mode)) {
            size_t size = s.st_size;
            char *data = (char*)mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
            if (MAP_FAILED != data) {
                reply->setStatusCode(200);
                reply->setBody(data, size);
                ok = true;
                munmap(data, size);
            } 
        }
        close(fd);
        if (!ok) {
            reply->setStatusCode(500);
            reply->setReasonPhrase("Internal Server Error");
        } else {
            size_t filenameLen = filename.length();
            size_t pos = filename.find_last_of(".");
            ANET_LOG(DEBUG,"filenameLen(%d),pos(%d)", filenameLen, pos);
            ANET_LOG(DEBUG,"sufix(%s)", filename.c_str() + pos);
            if (pos != string::npos && 
                (string(".htm") == string(filename.c_str() + pos)
                 || string(".html") == string(filename.c_str() + pos)))
            {
                reply->addHeader("Content-Type", "text/html");
            } else {
                //unknown Content-Type
                //reply->addHeader("Content-Type", "application/octet-stream");
            }
        }
        return;
    }
    
    void closeFile(const char*) {
        return;
    }
    RequestProcessor() {
        m_id = 0;
        m_runTime = 0;
        m_totalTime = 0;
        m_waitTime = 0;
        m_taskProcesssed = 0;
        m_lockTime = 0;
    }

    long m_id;
    int64_t m_runTime;
    int64_t m_totalTime;
    int64_t m_waitTime;
    int64_t m_taskProcesssed;
    int64_t m_lockTime;
};

void singalHandler(int seg)
{
//    ANET_LOG(INFO,"Singal(%d) received", seg);
    std::cerr << "Signal(" << seg << ") received!" << std::endl;
    globalStopFlag = true;
}

void doProcess(unsigned int port, unsigned int num);
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("%s rootdir port thread_num debug_level\n", argv[0]);
        return -1;
    }
    root = argv[1];
    unsigned int port = 8888;
    if (argc >= 3 && (port = atoi(argv[2])) < 1) {
        ANET_LOG(WARN, "Invalid port. Using default: 8888");
        port = 8888;
    } 
    unsigned int num = 1;
    if (argc >= 4 && (num = atoi(argv[3])) < 1) {
        ANET_LOG(WARN, "Invalid thread number. Using default: 1");
        num = 1;
    }
    int debugLevel = 0;
    if (argc >= 5) {
        debugLevel =atoi(argv[4]);
    }
    Logger::logSetup();
    Logger::setLogLevel(debugLevel);
    signal(SIGINT, singalHandler);
    signal(SIGTERM, singalHandler);

    doProcess(port, num);
    ANET_LOG(INFO, "exit.");
}

void doProcess(unsigned int port, unsigned int num) {
    Thread *threads = new Thread[num];
    RequestProcessor *runnables = new RequestProcessor [num];
    assert(threads);
    assert(runnables);
    for (long i=0; i<num; i++) {
        ANET_LOG(DEBUG, "Starting thread(%d)", i);
        threads[i].start(&runnables[i], (void*)i);
    }

    Transport transport;
//    transport.start(); //using multithreads mode of anet
    HTTPPacketFactory factory;
    HTTPStreamer streamer(&factory);
    HTTPServerAdapter serverAdapter;
    stringstream ss;
    char hostname[1024];
    if (gethostname(hostname, 1024) != 0) {
        ANET_LOG(ERROR, "Failed to get hostname");
        exit(-1);
    }
    ss << "tcp:" << hostname << ":" << port;
    string str = ss.str().c_str();
    const char *spec = str.c_str();
    ANET_LOG(INFO, "listen to %s", spec);
    IOComponent *ioc = transport.listen(spec, &streamer, &serverAdapter);
    if (ioc == NULL) {
        ANET_LOG(ERROR, "listen (%s) ERROR.", spec);
        return;
    }

    while (!globalStopFlag) {
//        usleep(100000);
        int64_t now;
        transport.runIteration(now);
    }
    ioc->subRef();
//    transport.stop();
//    transport.wait();
    transport.closeComponents();

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
    ANET_LOG(INFO, "Statistics Summary"); 
    ANET_LOG(INFO, "*****************************************************"); 
    ANET_LOG(INFO, "THREAD_ID TOT_TIME(ms) RUN_TIME(ms) LOCK_TIME(ms) WAIT_TIME(ms) TASKS_PROCESSED");
    for (long i=0; i<num; i++) {
        ANET_LOG(INFO, "%8ld %12.2f %12.2f %12.2f %13.2f %12ld",
                 runnables[i].m_id, 
                 runnables[i].m_totalTime / 1000.0, 
                 runnables[i].m_runTime / 1000.0, 
                 runnables[i].m_lockTime / 1000.0, 
                 runnables[i].m_waitTime / 1000.0, 
                 runnables[i].m_taskProcesssed);
    }
    delete [] threads;
    delete [] runnables;
}
