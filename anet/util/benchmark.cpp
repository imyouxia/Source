/**
 * The "helloworld_c_2" is a simple anet benchmarking tool
 */
#include <anet/anet.h>
#include <anet/log.h>
#include <anet/threadmutex.h>
#include <string>
#include <sstream>
#include <iostream>
#include <signal.h>
#include <fstream>

using namespace std;
using namespace anet;
struct RequestEntry {
    string mRequest;
    int64_t mSendTime;
    bool mReceived;
    bool mSuccess;
    size_t mReplyBytes;
};

class HelloWorldPacketHandler : public IPacketHandler {
public:
    IPacketHandler::HPRetCode
    handlePacket(Packet *packet, void *args) {
//        MutexGuarnd mutexGuard(&_mutex);
        _replyCount ++;
        RequestEntry *requestEntry = (RequestEntry*)args;
        requestEntry->mReceived = true;
        if (packet->isRegularPacket()) {
            requestEntry->mSuccess = true;
            DefaultPacket *reply = dynamic_cast<DefaultPacket*>(packet);
            requestEntry->mReplyBytes = reply->getBodyLen();
            ANET_LOG(DEBUG, "Request:|%s|,Reply:|%s|", 
                     requestEntry->mRequest.c_str(), reply->getBody());
        } else {
            requestEntry->mSuccess = false;
            ControlPacket *cmd = dynamic_cast<ControlPacket*>(packet);
            ANET_LOG(WARN, "Control Packet (%s) received!", cmd->what());
        }
        packet->free(); //free packet if finished
        return IPacketHandler::FREE_CHANNEL;
    }
    HelloWorldPacketHandler() {
        _replyCount = 0;
     }
public:
    int _replyCount;
//    ThreadMutex _mutex;
};

bool globalStopFlag = false;
void singalHandler(int seg)
{
//    ANET_LOG(INFO,"Singal(%d) received", seg);
    std::cerr << "Signal(" << seg << ") received!" << std::endl;
    globalStopFlag = true;
}

bool getLine(ifstream &infile, string &line, uint32_t retry = 1000u) {
    do {
        while (!infile.eof() && retry) {
            getline(infile, line);
            if ("" != line) {
                return true;
            }
            retry --;
        }
        ANET_LOG(DEBUG, "End of file! Read from head again");
        infile.clear();
        infile.seekg(0, ios::beg);
    } while (retry);
    return false;
}
int64_t globalCurrentPacketPosted = 0;
void post(Connection *conn, HelloWorldPacketHandler *handler,
          ifstream &in, RequestEntry *entry, int64_t now) 
{
    getLine(in, entry->mRequest);
    entry->mSendTime = now;
    entry->mReceived = false;
    DefaultPacket *packet = new DefaultPacket();
    packet->setBody(entry->mRequest.c_str(),
                    entry->mRequest.length() + 1);
    if (!conn->postPacket(packet, handler, entry)) {
        ANET_LOG(WARN, "Failed to send packet");
        entry->mReceived = true;
        entry->mSuccess = false;
        packet->free();
        if (conn->isClosed()) {
            globalStopFlag = true;
        }
    } else {
        globalCurrentPacketPosted ++;
    }
}

void receive(RequestEntry *entry, int64_t now, int64_t &total,
             int64_t &success, int64_t &fail, int64_t &minLatency,
             int64_t &maxLatency, int64_t &totalBytes)
{
    if (entry->mSuccess) {
        int64_t latency = now - entry->mSendTime;
        total += latency;
        success ++;
        minLatency = latency < minLatency ? latency : minLatency;
        maxLatency = latency > maxLatency ? latency : maxLatency;
        totalBytes += entry->mReplyBytes;
    } else {
        fail ++;
    }
    globalCurrentPacketPosted --;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("%s query_file spec parallel debug_level\n", argv[0]);
        exit(-1);
    }
    const char *queryFile = argv[1];
    const char *spec = argv[2];

    unsigned int parallel = 1;
    if (argc >= 4 && ((parallel = atoi(argv[3])) < 1 || parallel > 10000)) {
        ANET_LOG(WARN, "Invalid parallel (%u). Using default: 1", parallel);
        parallel = 1;
    }
    ANET_LOG(INFO, "query file:|%s|, spec:|%s|, parallel: %u",
             queryFile, spec, parallel);
    int debugLevel = 2;
    if (argc >= 5) {
        debugLevel =atoi(argv[4]);
    }
    Logger::logSetup();
    Logger::setLogLevel(debugLevel);
    signal(SIGINT, singalHandler);
    signal(SIGTERM, singalHandler);
    
    //char line[10240];
    ifstream infile(queryFile);
    if (!infile) {
        ANET_LOG(ERROR, "Failed to open file \"%s\"", queryFile);
        exit(-1);
    }
    if (EOF == infile.peek()) {
        ANET_LOG(ERROR, "Empty query File \"%s\"", queryFile);
        exit(-1);
    }

    DefaultPacketFactory factory;
    DefaultPacketStreamer streamer(&factory);
    HelloWorldPacketHandler handler;
    Transport transport;
    ANET_LOG(INFO,"Connecting to %s", spec);
    Connection *connection = transport.connect(spec, &streamer);
    if (!connection) {
        ANET_LOG(ERROR, "Fail to connect to %s", spec);
        exit(-1);
    }
    connection->setQueueLimit(parallel);
    RequestEntry *requests = new RequestEntry[parallel];
    int64_t now = TimeUtil::getTime();
    for (unsigned int i = 0; i < parallel; i ++) {
        post(connection, &handler, infile, &requests[i], now);
    }
    int64_t totalTime = 0;
    int64_t successCount = 0;
    int64_t failCount = 0;
    int64_t begin = TimeUtil::getTime();
    int64_t maxLatency = 0;
    int64_t minLatency = (int64_t)1 << 62;
    int64_t totalBytes = 0;
    while (!globalStopFlag) {
        transport.runIteration(now);
        for (unsigned int i = 0; i < parallel; i ++) {
            RequestEntry *entry = &requests[i];
            if (entry->mReceived) {
                receive(entry, now, totalTime, successCount, failCount,
                        minLatency, maxLatency, totalBytes);
                post(connection, &handler, infile, &requests[i], now); 
            }
        }
    }
    connection->close();//Close this connection.
    connection->subRef();//Do not use this connection any more.
    transport.closeComponents();
    for (unsigned int i = 0; i < parallel; i ++) {
        RequestEntry *entry = &requests[i];
        if (entry->mReceived) {
            receive(entry, now, totalTime, successCount, failCount,
                    minLatency, maxLatency, totalBytes);
        } else {
            assert(false);
        }
    }
    delete [] requests;
    int64_t end = TimeUtil::getTime();
    float totalRunTime = (end - begin) / 1000.0;
    cout << "****************************************" << endl;
    cout << "Total Time Used: " << totalRunTime << " ms" << endl;
    cout << "Success Request: " << successCount << endl;
    cout << "Fail    Request: " << failCount << endl;
    cout << "Bytes  Received: " << totalBytes << endl;
    cout << "Network Traffic: " << totalBytes * 1.0 / totalRunTime << " KB/s\n";
    cout << "Min     latency: " << minLatency / 1000. << " ms" << endl;
    cout << "Max     latency: " << maxLatency / 1000. << " ms" << endl;
    cout << "Average latency: " << totalTime / 1000. / successCount << " ms\n";
    cout << "Requests/second: " << successCount * 1000 / totalRunTime << endl;
    cout << "****************************************" << endl;
    ANET_LOG(INFO,"%s is about to exit!", argv[0]);
}

