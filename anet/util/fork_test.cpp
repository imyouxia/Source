#include <iostream>
#include <anet/anet.h>
#include <signal.h>
#include <sys/wait.h>
using namespace std;
using namespace anet;

DefaultPacketFactory factory;
DefaultPacketStreamer streamer(&factory);

const char *spec = "tcp:127.0.0.1:8912";
class EchoServer : public IServerAdapter 
{
public:
    void start()
    {
        _ioComponent = _transport.listen(spec, &streamer, this);
        assert(_ioComponent);

        _transport.start();
    }
    
    void stop() {
        _ioComponent->close();
        _ioComponent->subRef();
        _transport.stop();
        _transport.wait();
    }

    IPacketHandler::HPRetCode handlePacket(Connection *connection, Packet *packet) 
    {
        cerr << "packet received" << endl;
        if (packet->isRegularPacket()) {
            int pid = fork();
            if (pid == 0) {
                 cout << "a new child created." << endl;
                 if( -1 == execl("./wait", "./wait", "100", NULL)) {
                    cerr << "ERROR: " <<strerror(errno) << " " << errno << endl;
                      exit(1);
                  }
//                 for(;;) sleep(1);
//                 cerr << " created." << endl;

            }
            usleep(200000);
            connection->postPacket(packet);
            cerr << "packet processed " << getpid() <<endl;
        } else {
            packet->free();
        }
        return IPacketHandler::FREE_CHANNEL;
    }
    virtual ~EchoServer() {}

private:
    Transport _transport;
    IOComponent* _ioComponent;
};

class EchoClient : public IPacketHandler
{
public:
    HPRetCode handlePacket(Packet *packet, void *args)
    {
        packet->free();
        delete this;
        return IPacketHandler::FREE_CHANNEL;
    }

    void Run()
    {
        mConnection = mClientTrans->connect(spec, &streamer, false);
        assert(mConnection);

        const char* msg = "Hello!";
        DefaultPacket* packet = new DefaultPacket;
        packet->setBody(msg, strlen(msg));
        cout << "before postPacket() in client" << endl;
        mConnection->postPacket(packet, this, NULL);
        cout << "before usleep() in client" << endl;
        usleep(100000);
        cout << "before close() in client" << endl;
        mConnection->close();     
        cout << "before subRef() in client" << endl;
        mConnection->subRef();
        cout << "after subRef() in client" << endl;
    }
public:
    Transport *mClientTrans;
private:
    Connection* mConnection;
};

void sigHandler(int sig) {
    cout << "Signal Received: " << sig << endl;
}

int main()
{
    signal(SIGPIPE, sigHandler);
    signal(SIGHUP, sigHandler);
    Transport gClientTrans;
    gClientTrans.start();
    
    EchoServer server;
    server.start();

    for (int i = 0; i < 1; i++)
    {
        EchoClient* client = new EchoClient;        
        client->mClientTrans = &gClientTrans;
        cout << "Round " << i << endl;
        client->Run();
        usleep(100000);
        waitpid(-1, NULL, WNOHANG);
    }
    gClientTrans.stop();
    gClientTrans.wait();
    server.stop();
}
