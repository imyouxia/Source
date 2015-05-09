#include <iostream>
#include <string>
#include "sockettf.h"
#include <anet/anet.h>
#include <unistd.h>
#include <anet/serversocket.h>
#include <anet/log.h>
#include <signal.h>
using namespace std;
namespace anet {
CPPUNIT_TEST_SUITE_REGISTRATION(SocketTF);

class PlainConnectRunnable : public Runnable {
public:
    void run(Thread* thread, void *args) {
        Socket *socket = (Socket *) args;
        CPPUNIT_ASSERT(socket);
        CPPUNIT_ASSERT(socket->connect());
    }
};

struct SocketPair {
    ServerSocket * serverSocket;
    Socket *acceptedSocket;
};

class PlainServerRunnable : public Runnable {
public:
    void run(Thread* thread, void *args) {
        SocketPair *sockpair = (SocketPair*)args;
        CPPUNIT_ASSERT(sockpair->serverSocket);
        sockpair->acceptedSocket = sockpair->serverSocket->accept();
        CPPUNIT_ASSERT(sockpair->acceptedSocket);
    }
};

struct PlainReadArgs {
    Socket *socket;
    char * buffer;
    int bytes;
    int bytesRead;
};

class PlainReadRunnable : public Runnable {
public:
    void run(Thread* thread, void *args) {
        PlainReadArgs *myArgs = static_cast<PlainReadArgs*> (args);
        myArgs->bytesRead = myArgs->socket->read(myArgs->buffer,
                myArgs->bytes);
    }
};

void SocketTF::setUp() {
}
void SocketTF::tearDown() {
}

void SocketTF::testSetGetAddress() {
    Socket socket;
    char result[32];
    string expect;
    //testing invalid address
    CPPUNIT_ASSERT(!socket.setAddress("NoSushAddress.james.zhang",12345));
    CPPUNIT_ASSERT(socket.setAddress(NULL, 12345));
    CPPUNIT_ASSERT(socket.getAddr(result, 10));
    CPPUNIT_ASSERT_EQUAL(string("0.0.0.0:1"), string(result));
    CPPUNIT_ASSERT(socket.setAddress("", 0));
    CPPUNIT_ASSERT(socket.setAddress("localhost", 12345));
    CPPUNIT_ASSERT(socket.getAddr(result, 32));
    CPPUNIT_ASSERT_EQUAL(string("127.0.0.1:12345"), string(result));
    CPPUNIT_ASSERT(socket.setAddress("127.0.0.1", -1));
    CPPUNIT_ASSERT(socket.getAddr(result, 32));
    CPPUNIT_ASSERT_EQUAL(string("127.0.0.1:65535"), string(result));
    CPPUNIT_ASSERT(socket.setAddress("202.165.102.205", 12345));
    CPPUNIT_ASSERT(socket.setAddress("www.yahoo.com", 12345));
    CPPUNIT_ASSERT(socket.setAddress("g.cn", 12345));
}

void SocketTF::testReadWrite() {
    Socket socket;
    ServerSocket serverSocket;
    char data[]="Some Data";
    char output[1024];

    CPPUNIT_ASSERT_EQUAL(-1,socket.write(data, strlen(data)));
    CPPUNIT_ASSERT(socket.setAddress("localhost", 11234));
    CPPUNIT_ASSERT(serverSocket.setAddress("localhost", 11234));
    CPPUNIT_ASSERT(serverSocket.listen());
    SocketPair socketPair;
    socketPair.serverSocket=&serverSocket;
    Thread tc, ts;
    PlainConnectRunnable pcr;
    PlainServerRunnable psr;
    tc.start(&pcr,&socket);//connect
    ts.start(&psr,&socketPair);//accept
    ts.join();
    tc.join();
    Socket *acceptedSocket = socketPair.acceptedSocket;
    acceptedSocket->setSoBlocking(false);
    socket.setSoBlocking(false);
    CPPUNIT_ASSERT(acceptedSocket);
    CPPUNIT_ASSERT_EQUAL(9, socket.write(data, strlen(data)));
    CPPUNIT_ASSERT_EQUAL(9, acceptedSocket->read(output, 10));
    CPPUNIT_ASSERT_EQUAL(-1, acceptedSocket->read(NULL, 3));
    CPPUNIT_ASSERT_EQUAL(string(data, 9), string(output, 9));
    CPPUNIT_ASSERT_EQUAL(-1, acceptedSocket->read(output,10));
    CPPUNIT_ASSERT_EQUAL(EAGAIN, Socket::getLastError());
    CPPUNIT_ASSERT_EQUAL(3, socket.write(data, 3));
    CPPUNIT_ASSERT_EQUAL(3, acceptedSocket->read(output, 10));
    CPPUNIT_ASSERT_EQUAL(4, acceptedSocket->write(data, 4));
    CPPUNIT_ASSERT_EQUAL(4, socket.read(output, 10));
    CPPUNIT_ASSERT_EQUAL(string(data, 4), string(output, 4));
    CPPUNIT_ASSERT_EQUAL(-1, socket.write(NULL, 3));
    CPPUNIT_ASSERT_EQUAL(-1, acceptedSocket->read(NULL, 3));
    acceptedSocket->close();
    CPPUNIT_ASSERT_EQUAL(-1, acceptedSocket->read(output, 10));
    CPPUNIT_ASSERT_EQUAL(0, socket.read(output,3));
    /**@note: we can write to socket whose peer was closed*/
    CPPUNIT_ASSERT_EQUAL(3, socket.write(data,3));
    delete acceptedSocket;
    socket.close();
    CPPUNIT_ASSERT_EQUAL(-1, socket.write(data, 3));
    tc.start(&pcr,&socket);//connect
    ts.start(&psr,&socketPair);//accept
    ts.join();
    tc.join();
    acceptedSocket = socketPair.acceptedSocket;
    acceptedSocket->setSoBlocking(false);
    CPPUNIT_ASSERT(acceptedSocket);
    acceptedSocket->shutdown();
    CPPUNIT_ASSERT_EQUAL(-1, acceptedSocket->read(output, 10));
    signal(SIGPIPE, SIG_IGN);
    CPPUNIT_ASSERT_EQUAL(-1, acceptedSocket->write(data, 10));
    /**
     * @todo need to handle socket shutdown?
     * CPPUNIT_ASSERT_EQUAL(-1, acceptedSocket->read(output, 10));
     * CPPUNIT_ASSERT_EQUAL(-1, acceptedSocket->write(data, 10));
     */
    socket.close();
    delete acceptedSocket;
    CPPUNIT_ASSERT(socket.createUDP());
    CPPUNIT_ASSERT_EQUAL(-1, socket.write(data, 3));
    CPPUNIT_ASSERT(socket.setAddress("localhost",22));
    CPPUNIT_ASSERT(socket.connect());
    CPPUNIT_ASSERT_EQUAL(5, socket.write(data, 5));
    /**
     * @todo need more UDP interface
     * CPPUNIT_ASSERT_EQUAL(string("Need More"), string("UDP Interface"));
     */
    socket.close();
    serverSocket.close();
    acceptedSocket->close();
}

void SocketTF::testConnect() {
    char data[] = "Short Data";
    char output[256];
    Socket socket;

    CPPUNIT_ASSERT(!socket.connect());
    CPPUNIT_ASSERT(socket.setAddress("localhost",12346));
    CPPUNIT_ASSERT(!socket.connect());

    ServerSocket serverSocket;
    ServerSocket serverSocket2;

    CPPUNIT_ASSERT(!serverSocket.listen());
    CPPUNIT_ASSERT(serverSocket.setAddress("localhost",12346));
    CPPUNIT_ASSERT(serverSocket2.setAddress("localhost",12346));
    CPPUNIT_ASSERT(serverSocket.listen());
    CPPUNIT_ASSERT(serverSocket.setSoBlocking(false));
    CPPUNIT_ASSERT(!serverSocket2.listen());

    CPPUNIT_ASSERT(socket.connect());
    CPPUNIT_ASSERT(socket.setSoBlocking(false));
    /**
     * @todo should we detect if no body accept()?
     * CPPUNIT_ASSERT_EQUAL(-1, socket.write(data,3));
     */
    CPPUNIT_ASSERT_EQUAL(3, socket.write(data,3));

    Socket *acceptedSocket = serverSocket.accept();
    CPPUNIT_ASSERT(acceptedSocket);
    CPPUNIT_ASSERT(acceptedSocket->setSoBlocking(false));
    ANET_LOG(DEBUG,"Before acceptedSocket->read(output,256)");
    CPPUNIT_ASSERT_EQUAL(3, acceptedSocket->read(output,256));
    ANET_LOG(DEBUG,"After acceptedSocket->read(output,256)");
    CPPUNIT_ASSERT_EQUAL(string(data,3), string(output,3));
    ANET_LOG(DEBUG,"Before serverSocket.accept()");
    Socket *acceptedSocket2 = serverSocket.accept();
    ANET_LOG(DEBUG,"After serverSocket.accept()");
    CPPUNIT_ASSERT(!acceptedSocket2);
    delete acceptedSocket;
    acceptedSocket = NULL;
    CPPUNIT_ASSERT(socket.reconnect());
    acceptedSocket2 = serverSocket.accept();
    CPPUNIT_ASSERT(acceptedSocket2);
    CPPUNIT_ASSERT(acceptedSocket2->setSoBlocking(true));
    
    Thread thread;
    PlainReadRunnable readRunnable;
    PlainReadArgs args;
    args.socket = acceptedSocket2;
    args.buffer = output;
    args.bytes = 8;
    thread.start(&readRunnable, &args);

    socket.setTcpNoDelay(true);
    sleep(1);
    CPPUNIT_ASSERT_EQUAL(3, socket.write(data,3));
    CPPUNIT_ASSERT_EQUAL(5, socket.write(data+3, 5));
    thread.join();
    /**Blocking read will be blocked when there is no data to read!
     * So we should expect args.bytes > 0*/
    CPPUNIT_ASSERT(args.bytesRead > 0);
    CPPUNIT_ASSERT_EQUAL(string(data,args.bytesRead), 
                         string(output,args.bytesRead));
    delete acceptedSocket2;
}

void SocketTF::testListenZeroIPZeroPort() {
    ANET_LOG(DEBUG, "testListenZeroIPZeroPort");
    ServerSocket server;
    CPPUNIT_ASSERT(server.setAddress("0.0.0.0", 0));
    char result[32];
    CPPUNIT_ASSERT(server.getAddr(result, 32));
    CPPUNIT_ASSERT_EQUAL(string("0.0.0.0:0"), string(result));    
    CPPUNIT_ASSERT(!server.getAddr(result, 32, true));
    CPPUNIT_ASSERT(server.listen());
    CPPUNIT_ASSERT(server.getAddr(result, 32, true));
    ANET_LOG(DEBUG, "active address: %s", result);
    CPPUNIT_ASSERT(string("0.0.0.0:0") != string(result));
}

}
