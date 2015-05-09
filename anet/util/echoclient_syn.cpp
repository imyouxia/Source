#include <anet/anet.h>
#include <anet/log.h>
#include <signal.h>
#include <iostream>

using namespace anet;

bool gStop = false;

#define DATA_MAX_SIZE 4096

class EchoRunnable : public Runnable 
{
public:
    EchoRunnable() {
        _recvlen = 0; 
        _sendlen = 0;
        _count = 0;
        _errorcount = 0;
        _timeoutCount = 0;
        _closeCount = 0;
        _badCount = 0;
        _packetsToSend = 0;
        _connection = NULL;
        _tid = 0;
    }
    
    void doSend() {
        char buffer[DATA_MAX_SIZE+1];
        int len = 1988; //rand() % (DATA_MAX_SIZE-30) + 25;
        sprintf(buffer, "%010d_%010d", _tid, _count); 
        memset(buffer+21, 'a', len-21); 
        DefaultPacket *request = new DefaultPacket();
        request->setBody(buffer, len);
        Packet *packet = _connection->sendPacket(request);
        if (!packet) {
            ANET_LOG(ERROR, "Faild to send packet, index[%d:%d]", _tid, _count);
            request->free();
            gStop = true;
            return;
        } else if (packet && !packet->isRegularPacket()) { // 是否正常的包
            _errorcount++;
            ANET_LOG(ERROR, "=> ControlPacket: %d, index[%d:%d]", 
                     ((ControlPacket*)packet)->getCommand(), _tid, _count);
            switch(((ControlPacket*)packet)->getCommand()) {
            case ControlPacket::CMD_BAD_PACKET:
                _badCount++;
                break;
            case ControlPacket::CMD_TIMEOUT_PACKET:
                _timeoutCount++;
                break;
            case ControlPacket::CMD_CONNECTION_CLOSED:
                _closeCount++;
                break;
            }
        } else {
            _recvlen += ((DefaultPacket*)packet)->getBodyLen();
            packet->free();
        }
        _sendlen += len;
        _count ++;
    }

    void run(Thread *thread, void *args) {
        ANET_LOG(INFO, "thread %d started with %d to send.", 
                 _tid, _packetsToSend);
        while (!gStop && _count < _packetsToSend) {
            doSend();
        }
        ANET_LOG(INFO, "thread %d Quit.", _tid);
    }

    int _count;
    int _errorcount;
    int64_t _recvlen;
    int64_t _sendlen;
    int _timeoutCount;
    int _closeCount;
    int _badCount;
    int _packetsToSend;
    Connection *_connection;
    int _tid;
};

class EchoClient {
public:
    EchoClient(char *spec, int threads_count) {
        _spec = strdup(spec);
        _threads_count = threads_count;
        _recvlen = 0; 
        _sendlen = 0; 
        _count = 0;
        _errorcount = 0;
        _timeoutCount = 0;
        _closeCount = 0;
        _badCount = 0;
        _threads = NULL;
        _runnalbes = NULL;;
    }

    ~EchoClient() {
        if (_spec) {
            free(_spec);
        }
    }

    void start(int total, int c);
    char *_spec;
    int _threads_count;

    int _count;
    int _errorcount;
    int64_t _recvlen;
    int64_t _sendlen;
    int _timeoutCount;
    int _closeCount;
    int _badCount;
    Thread *_threads;
    EchoRunnable *_runnalbes;
};

void EchoClient::start(int total, int conncount) {
    DefaultPacketFactory factory ;
    DefaultPacketStreamer streamer(&factory);
    Transport transport;
    transport.start();

    Connection **cons = (Connection**) malloc(conncount*sizeof(Connection*));
    for(int i=0; i<conncount; i++) {
        cons[i] = transport.connect(_spec, &streamer, true);
        if (cons[i] == NULL) {
            ANET_LOG(ERROR, "connection error.");
            return;
        }
        cons[i]->setQueueLimit(_threads_count);
    }
    
    int totalThreads = _threads_count * conncount;
    int requestPerThread = (total + totalThreads - 1) / totalThreads;
    Thread *threads = new Thread[totalThreads];
    EchoRunnable *runnables = new EchoRunnable[totalThreads];
    for (int i = 0; i < conncount; i ++) {
        for (int j = 0; j < _threads_count; j ++) {
            int tid = i * _threads_count + j;
            runnables[tid]._tid = tid;
            runnables[tid]._connection = cons[i];
            runnables[tid]._packetsToSend = requestPerThread;
            threads[tid].start(&runnables[tid], NULL);
        }
    }

    for (int i = 0; i < conncount; i ++) {
        for (int j = 0; j < _threads_count; j ++) {
            int tid = i * _threads_count + j;
            threads[tid].join();
            _recvlen += runnables[tid]._recvlen; 
            _sendlen += runnables[tid]._sendlen; 
            _count += runnables[tid]._count;
            _errorcount += runnables[tid]._errorcount;
            _timeoutCount += runnables[tid]._timeoutCount;
            _closeCount += runnables[tid]._closeCount;
            _badCount += runnables[tid]._badCount;
        }
        cons[i]->close();
        cons[i]->subRef();
    }

    delete [] runnables;
    delete [] threads;
    free(cons);

    ANET_LOG(INFO,"Stoping Transport");
    transport.stop();
    transport.wait();
    ANET_LOG(DEBUG,"Transport Stopped");

    ANET_LOG(INFO,"Send  Count: %d, Send  Lengths: %ld", _count, _sendlen);
    ANET_LOG(INFO,"Reply Count: %d, Reply Lengths: %ld, Error Count: %d", 
             _count, _recvlen, _errorcount);
    ANET_LOG(INFO,"Bad Count: %d, Timeout Count: %d, Closed Count: %d",
             _badCount, _timeoutCount, _closeCount);
}

void singalHandler(int sig) {
    std::cerr << "Signal(" << sig << ") received!" << std::endl;
    gStop = true;
}

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 6) {
        printf("%s [tcp|udp]:ip:port count conn [threads per connection] [debug level]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int debugLevel = 0;
    if (6 == argc ) {
        debugLevel =atoi(argv[5]);
    }
    Logger::logSetup();
    Logger::setLogLevel(debugLevel);

    int sendcount = atoi(argv[2]);
    if (sendcount < 1) {
        sendcount = 1000;
        ANET_LOG(WARN, "Invalid count, use default: 1000");
    }
    int conncount = atoi(argv[3]);
    if (conncount < 1) {
        conncount = 1;
        ANET_LOG(WARN, "Invalid connection count, use default: 1");
    }
    
    int threads_count = 8;
    if (argc > 4 ) {
        threads_count = atoi(argv[4]);
    }
    if (threads_count < 1) {
        threads_count = 1;
        ANET_LOG(WARN, "Invalid threads per connection, use default: 1");
    }

    signal(SIGINT, singalHandler);
    signal(SIGTERM, singalHandler);
    signal(SIGPIPE, SIG_IGN);

    EchoClient echoclient(argv[1], threads_count);

    int64_t startTime = TimeUtil::getTime();
    echoclient.start(sendcount, conncount);
    int64_t endTime = TimeUtil::getTime();

    int64_t timeUsed = endTime - startTime;
    double seconds = timeUsed ? timeUsed / 1000000.0 : 1.0;

    ANET_LOG(INFO, "*****************************");
    ANET_LOG(INFO, "Time used %.2f ms", seconds * 1000);
    ANET_LOG(INFO, "Send Speed: %.2f tps, agv send size: %f",
             echoclient._count / seconds,
             echoclient._sendlen / (double)echoclient._count);

    ANET_LOG(INFO, "S&R  Speed: %.2f tps", echoclient._count / seconds);
    return 0;
}


