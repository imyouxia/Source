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
struct RequestEntry {
    Connection * _connection;
    DefaultPacket * _packet;
    RequestEntry() 
        : _connection(NULL),
          _packet(NULL),
          mStartPush(0),
          mStartWait(0),
          mStartProcess(0),
          mEndProcess(0) {}
    int64_t mStartPush;
    int64_t mStartWait;
    int64_t mStartProcess;
    int64_t mEndProcess;
};

class RequestQueue {
public:
    ThreadCond _condition;
    queue<RequestEntry> _queue;
    size_t mQueueLimit;
    size_t mWaitingWorkers;
    int64_t mEnqueueCount;
    int64_t mSignalCount;
    int64_t mAccEnqueueTime;
    int64_t mAccEnqueueWaitTime;
    int64_t mAccEnqueueLength;

    int64_t mDequeCount;
    int64_t mWaitCount;
    int64_t mAccDequeueTime;
    int64_t mAccDequeueWaitTime;
    int64_t mAccDequeueLength;
    RequestQueue()
        : mQueueLimit(200), 
          mWaitingWorkers(0),
          mEnqueueCount(0),
          mSignalCount(0),
          mAccEnqueueTime(0),
          mAccEnqueueWaitTime(0),
          mAccEnqueueLength(0),
          mDequeCount(0),
          mWaitCount(0),
          mAccDequeueTime(0),
          mAccDequeueWaitTime(0),
          mAccDequeueLength(0){}
} globalQueue;

string root = "/";
class MyServerAdapter : public IServerAdapter {
public:
    IPacketHandler::HPRetCode 
    handlePacket(Connection *connection, Packet *packet) {
        if (packet->isRegularPacket()) {//handle request
            DefaultPacket *request = dynamic_cast<DefaultPacket*>(packet);
            if (NULL == request) { 
                ANET_LOG(WARN, "Invalid DefaultPacket received");
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

    void myHandlePacket(Connection *connection, DefaultPacket* request) {
        int64_t begin = TimeUtil::getTime();
        globalQueue._condition.lock();
        int64_t afterLock = TimeUtil::getTime();
        globalQueue.mAccEnqueueLength += globalQueue._queue.size();
        if (globalQueue._queue.size() >= globalQueue.mQueueLimit) {
            ANET_LOG(WARN,"Service Unavailable");
            request->setBody("Service Unavailable",20);
            if (!connection->postPacket(request)) {
                ANET_LOG(ERROR,"Faild to send reply packet!");
                request->free();
            }
        } else {//add connection, request to globalQueue for later processing
            RequestEntry entry;
            connection->addRef();//add a reference count for later using
            entry._connection = connection;
            entry._packet = request;
            entry.mStartPush = begin;
            entry.mStartWait = TimeUtil::getTime();
            globalQueue._queue.push(entry);
            if (globalQueue.mWaitingWorkers > 0) {
                globalQueue.mSignalCount ++;
                globalQueue._condition.signal();
            }
        }
        int64_t end = TimeUtil::getTime();
        globalQueue.mEnqueueCount ++;
        globalQueue.mAccEnqueueWaitTime += afterLock - begin;
        globalQueue.mAccEnqueueTime += end  - begin;
        globalQueue._condition.unlock();
    }//End of myHandlePacket(Connection *connection, DefaultPacket* request)
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
            globalQueue.mWaitingWorkers ++;
            globalQueue._condition.wait();
            globalQueue.mWaitingWorkers --;
            globalQueue.mWaitCount ++;
        }
        int64_t afterWait = TimeUtil::getTime();

        m_waitTime += afterWait - afterLock;
        if (globalStopFlag) {
            ANET_LOG(DEBUG,"Thread (%d) Stopped!", m_id);
            globalQueue._condition.unlock();
            return;
        }
        RequestEntry entry = globalQueue._queue.front();
        globalQueue._queue.pop();
        int64_t afterPop = TimeUtil::getTime();
        globalQueue.mDequeCount ++;
        globalQueue.mAccDequeueWaitTime += afterWait - begin;
        globalQueue.mAccDequeueTime += afterPop - begin;
        entry.mStartProcess = afterPop;
        globalQueue.mAccDequeueLength += globalQueue._queue.size();
        globalQueue._condition.unlock();
        m_taskWaitTime += entry.mStartProcess - entry.mStartWait;

        for (int i = 0; i < 1024 * 1024 * 5; i ++) {
            //dummy processing!
        }
        doReply(entry);

        m_taskProcesssed ++;
        int64_t end = TimeUtil::getTime();
        m_runTime += end - afterPop;
        m_taskProcessTime += end - afterPop;
    }

    void doReply(RequestEntry &entry) {
        ANET_LOG(SPAM, "Processing Request");
        DefaultPacket *reply = entry._packet;
        size_t length;
        const char *tmp = entry._packet->getBody(length);
        if (!tmp || !length || tmp[0] != '/') {
            ANET_LOG(WARN, "invalid URI");
            reply->setBody("Invalid URI!", 13);
        } else {
            string uri(tmp, length);
            if (uri[uri.length() - 1] == '/') {
                uri +="index.html";
            }
            fillBody(root + uri, reply);
        }
        if (!entry._connection->postPacket(reply)) {
            if (entry._connection->isClosed()) {
                ANET_LOG(WARN, "Failed to send reply! Connection closed!");
            } else {
                assert(false);
            }
            reply->free();
        }
        entry._connection->subRef();
    }

    void fillBody(const string &filename, DefaultPacket *reply) {
        int fd = open(filename.c_str(), O_RDONLY);
        if ( -1 == fd ) {
            ANET_LOG(WARN, "file '%s' Not Found!", filename.c_str());
            reply->setBody("Not Found!", 11);
            return;
        }
        bool ok = false;
        struct stat s;
        if (-1 != fstat(fd, &s) && S_ISREG(s.st_mode)) {
            size_t size = s.st_size;
            char *data = (char*)mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
            if (MAP_FAILED != data) {
                reply->setBody(data, size);
                ok = true;
                munmap(data, size);
            } 
        }
        close(fd);
        if (!ok) {
            reply->setBody("Internal Server Error", 22);
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
        m_taskWaitTime = 0;
        m_taskProcessTime = 0;
        m_taskProcesssed = 0;
        m_lockTime = 0;
    }

    long m_id;
    int64_t m_runTime;
    int64_t m_totalTime;
    int64_t m_waitTime;
    int64_t m_taskWaitTime;
    int64_t m_taskProcessTime;
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
    int debugLevel = 2;
    if (argc >= 5) {
        debugLevel =atoi(argv[4]);
    }
    Logger::logSetup();
    Logger::setLogLevel(debugLevel);
    signal(SIGINT, singalHandler);
    signal(SIGTERM, singalHandler);
    signal(SIGPIPE, SIG_IGN);
    
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
    DefaultPacketFactory factory;
    DefaultPacketStreamer streamer(&factory);
    MyServerAdapter serverAdapter;
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
//    transport.stop();
//    transport.wait();
    transport.closeComponents();

    globalQueue._condition.lock();
    while (globalQueue._queue.size()) {
        RequestEntry entry = globalQueue._queue.front();
        DefaultPacket *packet = (DefaultPacket*)entry._packet;
        ANET_LOG(WARN,"Discard request %s", packet->getBody());
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

    printf("*****************************************************\n"); 
    printf("Worker Summary\n"); 
    printf("THREAD_ID TOT_TIME(ms) LOCK_TIME(ms) TASK_TIME(ms) TASK_WAIT(ms) TASKS_PROCESSED\n");
    for (long i=0; i<num; i++) {
        printf("%8ld %12.2f %12.2f %12.2f %13.2f %12ld\n",
                 runnables[i].m_id, 
                 runnables[i].m_totalTime / 1000.0, 
                 runnables[i].m_lockTime / 1000.0, 
                 runnables[i].m_taskProcessTime / 1000.0, 
                 runnables[i].m_taskWaitTime / 1000.0, 
                 runnables[i].m_taskProcesssed);
    }
    printf("\n");
    printf("Queue Summary\n");
    printf("     #Enqueue Acc_Enqueue_Len #Signals Push_Time(ms) Push_Wait_Time(ms)\n"); 
    printf("%12ld %14ld %8ld %13.2f %18.2f\n",
           globalQueue.mEnqueueCount, 
           globalQueue.mAccDequeueLength,
           globalQueue.mSignalCount,
           globalQueue.mAccEnqueueTime / 1000.0,
           globalQueue.mAccEnqueueWaitTime / 1000.0);
    
    printf("     #Dequeue Acc_Dequeue_Len   #Waits  Pop_Time(ms)  Pop_Wait_Time(ms)\n"); 
    printf("%12ld %14ld %8ld %13.2f %18.2f\n",
           globalQueue.mDequeCount, 
           globalQueue.mAccDequeueLength,
           globalQueue.mWaitCount,
           globalQueue.mAccDequeueTime / 1000.0,
           globalQueue.mAccDequeueWaitTime / 1000.0);
    printf("*****************************************************\n"); 
    
    delete [] threads;
    delete [] runnables;
}
