#include <anet/log.h>
#include <anet/anet.h>
#include <anet/stats.h>
#include <signal.h>
#include <iostream>
using namespace anet;

int gsendcount = 1000;
int64_t gsendlen = 0;
int grecvcount = 0;
int grecvlen =0;
int gerrorcount = 0;
int gtimeoutcount = 0;

bool gStop = false;
Transport transport;

#define DATA_MAX_SIZE 4096

class EchoPacketHandler : public IPacketHandler
{
public:
    EchoPacketHandler() {
        _recvlen = 0; 
        _endTime = 0;
        atomic_set(&_count, 0);
        atomic_set(&_errorcount, 0);
        atomic_set(&_timeoutCount, 0);
        atomic_set(&_closeCount, 0);
        atomic_set(&_badCount, 0);
    }
    HPRetCode handlePacket(Packet *packet, void *args)
    {
        atomic_inc(&_count);
        if (!packet->isRegularPacket()) { // 是否正常的包
            atomic_inc(&_errorcount);
            ANET_LOG(ERROR, "=> ControlPacket: %d, index:%d", 
                     ((ControlPacket*)packet)->getCommand(), (long)args);
            switch(((ControlPacket*)packet)->getCommand()) {
            case ControlPacket::CMD_BAD_PACKET:
                atomic_inc(&_badCount);
                break;
            case ControlPacket::CMD_TIMEOUT_PACKET:
                atomic_inc(&_timeoutCount);
                break;
            case ControlPacket::CMD_CONNECTION_CLOSED:
                atomic_inc(&_closeCount);
                break;
            default:
                break;
            }
        } else {
            _recvlen += ((DefaultPacket*)packet)->getBodyLen();
            packet->free();
        }
        //int index = (int)args;
        _endTime = TimeUtil::getTime();
        if (_count.counter == gsendcount) {
            gStop = true;
        } 
        return IPacketHandler::FREE_CHANNEL;
    }    

    int64_t _endTime;
    atomic_t _count;
    atomic_t _errorcount;
    int64_t _recvlen;
    atomic_t _timeoutCount;
    atomic_t _closeCount;
    atomic_t _badCount;
};
EchoPacketHandler handler;

class EchoClient : public Runnable {
public:
    EchoClient(char *spec);
    ~EchoClient();
    void run(Thread* thread, void *arg);
    void start(int c);
    int64_t _startTime;
    int64_t _endTime;
    char *_spec;
    int64_t runTime(){
        return _endTime - _startTime;
    }
};

EchoClient::EchoClient(char *spec)
{
    _spec = strdup(spec);
    _startTime = _endTime = 0;
}

EchoClient::~EchoClient()
{
    if (_spec) {
        free(_spec);
    }
}

void EchoClient::run(Thread *thread, void *arg) {
    int *count = (int *) arg;
    start(*count);
}
DefaultPacketFactory factory ;
DefaultPacketStreamer streamer(&factory);
void EchoClient::start(int conncount)
{
    Connection **cons = (Connection**) malloc(conncount*sizeof(Connection*));
    for(int i=0; i<conncount; i++) {
        cons[i] = transport.connect(_spec, &streamer, true);
        if (cons[i] == NULL) {
            ANET_LOG(ERROR, "connection error.");
            return;
        }
        cons[i]->setDefaultPacketHandler(&handler);
        cons[i]->setQueueLimit(50lu);
    }
    ANET_LOG(DEBUG,"Before Transport Start()");
    transport.start();
    char buffer[DATA_MAX_SIZE+1];
    int sendcount = 0;
    int pid = getpid();
    ANET_LOG(INFO, "PID: %d", pid);
    _startTime = TimeUtil::getTime();
    for(int i=0; i<gsendcount; i++) {
        int len = 1988; //rand() % (DATA_MAX_SIZE-30) + 25;
        sprintf(buffer, "%010d_%010d", pid, i); 
        memset(buffer+21, 'a', len-21); 
        DefaultPacket *packet = new DefaultPacket();
        packet->setBody(buffer, len);
        system("sleep 0&");
        if (!cons[i%conncount]->postPacket(packet, NULL, (void*)((long)i),true)) {
            ANET_LOG(SPAM,"postPacket() Failed! IOC(%p)", cons[i%conncount]);
            break;
        }
//        kill(getpid(), SIGCHLD);
        gsendlen += len;
        sendcount ++;
    }
    gsendcount = sendcount;
    ANET_LOG(INFO, "send finish.");
    _endTime = TimeUtil::getTime();
    for(int i=0; i<conncount; i++) {
        cons[i]->subRef();
    }
    free(cons);
}

void singalHandler(int sig)
{
//    ANET_LOG(INFO,"Singal (%d) Caught!", sig);
    std::cerr << "Signal(" << sig << ") received!" << std::endl;
    gStop = true;
}

int main(int argc, char *argv[]) {
    if (argc < 4 || argc > 5) {
        printf("%s [tcp|udp]:ip:port count conn [debug level]\n", argv[0]);
        return EXIT_FAILURE;
    }
    int sendcount = atoi(argv[2]);
    if (sendcount >= 0) {
        gsendcount = sendcount;
    }
    int conncount = atoi(argv[3]);
    if (conncount < 1) {
        conncount = 1;
    }
    
    int debugLevel = 0;
    if (5 == argc ) {
        debugLevel =atoi(argv[4]);
    }
    Logger::logSetup();
    Logger::setLogLevel(debugLevel);
    signal(SIGINT, singalHandler);
    signal(SIGTERM, singalHandler);

    srand(time(NULL));
    EchoClient echoclient(argv[1]);
    Thread thread;
    thread.start(&echoclient, &conncount);
    while (!gStop) {
        usleep(100000);
    }
    ANET_LOG(INFO,"Stoping Transport");
    transport.stop();
    transport.wait();
    ANET_LOG(DEBUG,"Transport Stopped");
    thread.join();
    
    ANET_LOG(INFO,"Send  Count: %d, Send  Lengths: %lld", gsendcount, gsendlen);
    ANET_LOG(INFO,"Reply Count: %d, Reply Lengths: %lld, Error Count: %d", 
             handler._count.counter, handler._recvlen, 
             handler._errorcount.counter);
    ANET_LOG(INFO,"Bad Count: %d, Timeout Count: %d, Closed Count: %d",
             handler._badCount.counter, handler._timeoutCount.counter,
             handler._closeCount.counter);

    int64_t endTime = ( echoclient._endTime > handler._endTime ) 
                      ? echoclient._endTime
                      : handler._endTime;
    endTime -= echoclient._startTime;
    endTime = endTime ? endTime : 1;
    ANET_LOG(INFO, "Send Speed: %d tps, agv send size: %d",
             (int)((1000000LL * gsendcount)
                   / ( echoclient.runTime() ? echoclient.runTime() : 1)),
             (int)(gsendlen/( gsendcount ? gsendcount : 1)));

    ANET_LOG(INFO, "S&R  Speed: %d tps",
             (int)((1000000LL * handler._count.counter)/ endTime));

    ANET_GLOBAL_STAT.log();
    
    return EXIT_SUCCESS;
}



