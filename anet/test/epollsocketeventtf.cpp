#include <anet/anet.h>
#include <string>
#include "epollsocketeventtf.h"
#include <unistd.h>
#include <anet/log.h>

using namespace std;

namespace anet {
CPPUNIT_TEST_SUITE_REGISTRATION(EPollSocketEventTF);

class EventRunnable : public Runnable {
public:
    EventRunnable():_rc(false) {}
public:
    bool _rc;
    SocketEvent *_socketEvent;
    Socket *_socket;
    bool _read;
    bool _write;
};

class AddEventRunnable : public EventRunnable {
public :
    void run(Thread* thread, void *args) {
      _rc = _socketEvent->addEvent(_socket, _read, _write);
    }
};

class SetEventRunnable : public EventRunnable {
public :
    void run(Thread* thread, void *args) {
        _setEventTime = TimeUtil::getTime();
      _rc = _socketEvent->setEvent(_socket, _read, _write);
    }
    int64_t _setEventTime;
};

EPollSocketEventTF::EPollSocketEventTF() {
    _tcpAccepted = NULL;
    _dummyClient = (IOComponent*)0xdc1e;
    _dummyListener = (IOComponent*)0xd1ee;
    _dummyAccepted = (IOComponent*)0xdacc;
}

void EPollSocketEventTF::setUp() {
    _tcpClient.setAddress("localhost", 11124);
    _tcpListener.setAddress("localhost", 11124);
    CPPUNIT_ASSERT(_tcpListener.listen());
    CPPUNIT_ASSERT(_tcpClient.connect());
    _tcpAccepted = _tcpListener.accept();
    CPPUNIT_ASSERT(_tcpAccepted);
    CPPUNIT_ASSERT(_tcpClient.setSoBlocking(false));
    CPPUNIT_ASSERT(_tcpListener.setSoBlocking(false));
    CPPUNIT_ASSERT(_tcpAccepted->setSoBlocking(false));
    _tcpClient.setIOComponent(_dummyClient);
    _tcpListener.setIOComponent(_dummyListener);
    _tcpAccepted->setIOComponent(_dummyAccepted);
}

void EPollSocketEventTF::tearDown() {
    _tcpClient.close();
    _tcpListener.close();
    if (_tcpAccepted) {
        delete _tcpAccepted;
    }
    _tcpAccepted = NULL;
}

void EPollSocketEventTF::testAddEvent() {
    Socket socket;
    EPollSocketEvent socketEvent;
    char readBuf[512];
    char data[512]="123456789abcdef0";
    IOEvent events[8];

    socket.setAddress("localhost",12345);
    //invalid socket
    CPPUNIT_ASSERT(!socketEvent.addEvent(&socket, true, false));

    //test read event
    CPPUNIT_ASSERT(socketEvent.addEvent(&_tcpClient, true, false));
    int64_t t1 = TimeUtil::getTime();
    CPPUNIT_ASSERT_EQUAL(0, socketEvent.getEvents(50, events, 8));
    int64_t t2 = TimeUtil::getTime();
    ANET_LOG(DEBUG, "Time used for getEvents():%d", t2 - t1);
    CPPUNIT_ASSERT((t2-t1) >= 40000);

    //add another event
    CPPUNIT_ASSERT(socketEvent.addEvent(_tcpAccepted, false, true));
    CPPUNIT_ASSERT_EQUAL(16, _tcpAccepted->write(data, 16));
    t1 = TimeUtil::getTime();
    CPPUNIT_ASSERT_EQUAL(2, socketEvent.getEvents(50, events, 8));
    t2 = TimeUtil::getTime();
    ANET_LOG(DEBUG, "Time used for getEvents():%d", t2 - t1);
    CPPUNIT_ASSERT((t2-t1) <= 10000);
    if (events[0]._ioc == _dummyClient) {
        CPPUNIT_ASSERT(events[0]._readOccurred);
        CPPUNIT_ASSERT(!events[0]._writeOccurred);
        CPPUNIT_ASSERT_EQUAL(_dummyAccepted, events[1]._ioc);
        CPPUNIT_ASSERT(events[1]._writeOccurred);
        CPPUNIT_ASSERT(!events[1]._readOccurred);
    } else if (events[1]._ioc == _dummyClient) {
        CPPUNIT_ASSERT(events[1]._readOccurred);
        CPPUNIT_ASSERT(!events[1]._writeOccurred);
        CPPUNIT_ASSERT_EQUAL(_dummyAccepted, events[0]._ioc);
        CPPUNIT_ASSERT(events[0]._writeOccurred);
        CPPUNIT_ASSERT(!events[0]._readOccurred);
    } else {
        CPPUNIT_ASSERT(!"Unknow IOEvent Got!");
    }
    CPPUNIT_ASSERT_EQUAL(16, _tcpClient.read(readBuf, 16));
    CPPUNIT_ASSERT_EQUAL(string(data, 16), string(readBuf, 16));
}

void EPollSocketEventTF::testClose() {
    EPollSocketEvent socketEvent;
    IOEvent events[8];
    int64_t t1,t2;
    CPPUNIT_ASSERT(socketEvent.addEvent(&_tcpClient, true, true));
    CPPUNIT_ASSERT(socketEvent.addEvent(_tcpAccepted, true, true));
    _tcpClient.close();
    t1 = TimeUtil::getTime();
    CPPUNIT_ASSERT_EQUAL(1, socketEvent.getEvents(50, events, 8));
    t2 = TimeUtil::getTime();
    ANET_LOG(DEBUG, "Time used for getEvents():%d", t2 - t1);
    CPPUNIT_ASSERT((t2-t1) <= 1000);

    CPPUNIT_ASSERT_EQUAL(_dummyAccepted, events[0]._ioc);
    CPPUNIT_ASSERT(events[0]._readOccurred);
    CPPUNIT_ASSERT(events[0]._writeOccurred);
    CPPUNIT_ASSERT(!events[0]._errorOccurred);
}

void EPollSocketEventTF::testEmptyEvent() {
    EPollSocketEvent socketEvent;    
    IOEvent events[8];

    //empty events
    int64_t t1 = TimeUtil::getTime();
    CPPUNIT_ASSERT_EQUAL(0, socketEvent.getEvents(100, events, 8));
    int64_t t2 = TimeUtil::getTime();
    ANET_LOG(DEBUG, "Time used for getEvents():%d", t2 - t1);
    CPPUNIT_ASSERT((t2-t1) >= 50000);

    Thread thread;
    AddEventRunnable runnable;
    runnable._socketEvent = &socketEvent;
    runnable._socket = &_tcpClient;
    runnable._read = false;
    runnable._write = true;
    thread.start(&runnable, NULL);
    t1 = TimeUtil::getTime();
    CPPUNIT_ASSERT_EQUAL(1, socketEvent.getEvents(50, events, 8));
    t2 = TimeUtil::getTime();
    ANET_LOG(DEBUG, "Time used for getEvents():%d", t2 - t1);
    CPPUNIT_ASSERT((t2-t1) <= 40000);
    thread.join();
    CPPUNIT_ASSERT(runnable._rc);
}

void EPollSocketEventTF::testSetEvent() {
    EPollSocketEvent socketEvent;    
    IOEvent events[8];
    int64_t t1,t2;

    CPPUNIT_ASSERT(socketEvent.addEvent(&_tcpClient, false, false));
 
    Thread thread;
    SetEventRunnable runnable;
    runnable._socketEvent = &socketEvent;
    runnable._socket = &_tcpClient;
    runnable._read = false;
    runnable._write = true;

    thread.start(&runnable, NULL);
    t1 = TimeUtil::getTime();
    CPPUNIT_ASSERT_EQUAL(1, socketEvent.getEvents(100, events, 8));
    t2 = TimeUtil::getTime();
    ANET_LOG(DEBUG, "Time used for getEvents():%d", t2 - t1);
    CPPUNIT_ASSERT((t2-t1) <= 50000);
    CPPUNIT_ASSERT(t2 >= runnable._setEventTime);
    ANET_LOG(DEBUG, "Time used for wake after setEvents():%d", 
             t2 - runnable._setEventTime);
    CPPUNIT_ASSERT(t2 - runnable._setEventTime < 50000);
    thread.join();
    CPPUNIT_ASSERT(runnable._rc);
    
    t1 = TimeUtil::getTime();
//    CPPUNIT_ASSERT_EQUAL(0, socketEvent.getEvents(100, events, 8));
    CPPUNIT_ASSERT_EQUAL(1, socketEvent.getEvents(100, events, 8));
    t2 = TimeUtil::getTime();
    ANET_LOG(DEBUG, "Time used for getEvents():%d", t2 - t1);
//    CPPUNIT_ASSERT((t2-t1) >= 50000);
    CPPUNIT_ASSERT((t2-t1) <= 50000);

    CPPUNIT_ASSERT(socketEvent.setEvent(&_tcpClient, false, true));
    CPPUNIT_ASSERT_EQUAL(1, socketEvent.getEvents(50, events, 8));

    //setEvent for socket that is not in the socketevent
    CPPUNIT_ASSERT(!socketEvent.setEvent(_tcpAccepted, false, true));
}

void EPollSocketEventTF::testRemoveEvent() {
    EPollSocketEvent socketEvent;    
    IOEvent events[8];

    CPPUNIT_ASSERT(socketEvent.addEvent(&_tcpClient, false, true));
    int64_t t1 = TimeUtil::getTime();
    CPPUNIT_ASSERT_EQUAL(1, socketEvent.getEvents(50, events, 8));
    int64_t t2 = TimeUtil::getTime();
    ANET_LOG(DEBUG, "Time used for getEvents():%d", t2 - t1);
    CPPUNIT_ASSERT((t2-t1) <= 40000);

    CPPUNIT_ASSERT(socketEvent.removeEvent(&_tcpClient));
    t1 = TimeUtil::getTime();
    CPPUNIT_ASSERT_EQUAL(0, socketEvent.getEvents(50, events, 8));
    t2 = TimeUtil::getTime();
    ANET_LOG(DEBUG, "Time used for getEvents():%d", t2 - t1);
    CPPUNIT_ASSERT((t2-t1) >= 40000);

    CPPUNIT_ASSERT(!socketEvent.removeEvent(&_tcpClient));
}

class WakeUpEventRunnable : public Runnable {
public:
    void run(Thread* thread, void *args) {
        EPollSocketEvent *myArgs = static_cast<EPollSocketEvent*>(args);
        usleep(20000);
        _wakeupTime = TimeUtil::getTime();
        myArgs->wakeUp();
    }
public:
    int64_t _wakeupTime;
};

void EPollSocketEventTF::testWakeUp() {
    EPollSocketEvent socketEvent;    
    IOEvent events[8];
    Thread thread;
    WakeUpEventRunnable runnable;

    //No other event in socketEvent
    thread.start(&runnable, &socketEvent);
    int64_t t1 = TimeUtil::getTime();
    CPPUNIT_ASSERT_EQUAL(0, socketEvent.getEvents(250, events, 8));
    int64_t t2 = TimeUtil::getTime();
    thread.join();
    ANET_LOG(DEBUG, "Time used for getEvents():%d", t2 - t1);
    CPPUNIT_ASSERT((t2-t1) <= 248000);
    ANET_LOG(DEBUG, "Time used for wakeUp():%d", t2 - runnable._wakeupTime);
    CPPUNIT_ASSERT(t2 >= runnable._wakeupTime);
    CPPUNIT_ASSERT((t2 - runnable._wakeupTime) <= 100000);

    //add an event to socketEvent
    CPPUNIT_ASSERT(socketEvent.addEvent(&_tcpClient, true, false));
    thread.start(&runnable, &socketEvent);
    t1 = TimeUtil::getTime();
    CPPUNIT_ASSERT_EQUAL(0, socketEvent.getEvents(250, events, 8));
    t2 = TimeUtil::getTime();
    thread.join();
    ANET_LOG(DEBUG, "Time used for getEvents():%d", t2 - t1);
    CPPUNIT_ASSERT((t2-t1) <= 248000);
    ANET_LOG(DEBUG, "Time used for wakeUp():%d", t2 - runnable._wakeupTime);
    CPPUNIT_ASSERT(t2 >= runnable._wakeupTime);
    CPPUNIT_ASSERT((t2 - runnable._wakeupTime) <= 100000);
}

void EPollSocketEventTF::testDup() {
    EPollSocketEvent socketEvent;    
    IOEvent events[8];
    int fd = dup(_tcpClient.getSocketHandle());// epoll deal with *file* not fd
    CPPUNIT_ASSERT(socketEvent.addEvent(&_tcpClient, false, true));
    CPPUNIT_ASSERT_EQUAL(1, socketEvent.getEvents(250, events, 8));    
    _tcpClient.close();
    CPPUNIT_ASSERT_EQUAL(1, socketEvent.getEvents(250, events, 8));
    CPPUNIT_ASSERT(events[0]._writeOccurred);
    CPPUNIT_ASSERT_EQUAL(_dummyClient, events[0]._ioc);
    close(fd);
}

}
