#ifndef ANET_TRANSPORT_H_
#define ANET_TRANSPORT_H_
#include <queue>
#include <vector>
#include <anet/thread.h>
#include <anet/epollsocketevent.h>
#include <anet/ipacketstreamer.h>
#include <anet/iserveradapter.h>
#include <anet/connection.h>
namespace anet {

/**
 * This class controls behavior of ANET. There are two work modes: 
 * Multi-Thread and Single-Thread. In Multi-Thread: socket read/write
 * and timeout checking working in different threads. In 
 * Single-Thread, all these jobs working in single threads.
 * When using Multi-Thread, its recommend to start by start(bool) 
 * and stop by stop(). When using Single-Thread, its recommend
 * to call runIteration() explicit. (More samples in anet/util/)
 */
class Transport : public Runnable {
    friend class TransportTF;
    friend class TCPCONNECTIONTF;
    friend class TCPCOMPONENTTF;
    friend class ConnectionTF;
public:
    enum CommandType {
        TC_ADD_IOC = 1,
        TC_REMOVE_IOC
    };

    struct TransportCommand {
        CommandType type;
        IOComponent *ioc;
    };

    Transport();

    ~Transport();

    /**
     * Run ANET in multi thread.
     * It will start two threads, one for read/write, one for 
     * timeout checking
     *
     * @param promotePriority promote the proproty of ANET thread
     * @return Return true if start success, else return false.
     */
    bool start(bool promotePriority = false);

    /**
     * Stop ANET. It will stop all threads of ANET.
     * In this function, _stopMutex will be locked.
     *
     * @return Return true if stop success, else return false. 
     */
    bool stop();

    /**
     * Waiting for read/write thread and timeout checking thread 
     * exit. It's not used in single thread mode.
     *
     * @return Return true if wait success, else return false.
     */
    bool wait();

    /**
     * Start thread.
     *
     * @param arg: args for thread
     */
    void run(Thread *thread, void *arg);

    /**
     * Run iteration in single thread. In this function, 
     * eventIteration and timeoutIteration will be called.
     *
     * @param now: execute time
     */
    void runIteration(int64_t &now);

    /**
     * Start anet as single thread. In this function, runTteration()
     * will be called. When start ANET using run(), ANET will run
     * in single thread mode. Both eventIteation and timeoutIteration
     * will work in one thread under this mode.
     */
    void run();

    /**
     * Stop anet when using single thread. It's very simple in this 
     * function. Just modigy _stop to true.
     */
    void stopRun();

    /**
     * 起一个监听端口。
     *
     * @param spec: 格式 [upd|tcp]:ip:port
     * @param streamer: 数据包的双向流，用packet创建，解包，组包。
     * @param serverAdapter: 用在服务器端，当Connection初始化及Channel创建时回调时用
     * @param timeout: set timeout in millisecond for server sending replies
     * @return IO组件一个对象的指针
     */
    IOComponent *listen(const char *spec,
                        IPacketStreamer *streamer, 
                        IServerAdapter *serverAdapter,
			int timeout = 5000);

    /**
     * 创建一个Connection，连接到指定的地址，并加入到Socket的监听事件中。
     *
     * @param spec: 格式 [upd|tcp]:ip:port
     * @param streamer: 数据包的双向流，用packet创建，解包，组包。
     * @param autoReconn: 是否重连
     * @return  返回一个Connectoion对象指针
     */
    Connection *connect(const char *spec, IPacketStreamer *streamer, bool autoReconn = false);

    /**
     * 加入到iocomponents中
     * 
     * @param  ioc: IO组件
     */
    void addComponent(IOComponent *ioc);

    /**
     * 从iocomponents中删除掉
     *
     * @param ioc: IO组件
     */
    void removeComponent(IOComponent *ioc);

    void postCommand(const CommandType type, IOComponent * ioc);

    /** 
     * Add ioc to timeout thread's checking list.
     *
     * @param ioc IOComponent to be checked periodically by 
     * timeout thread
     */
    void addToCheckingList(IOComponent *ioc);
    void collectNewComponets();
    SocketEvent* getSocketEvent();

    void lock();
    void unlock();

    /**
     * one iteration of processing socket related events
     * @param now current time in microsecond
     */
    void eventIteration(int64_t &now);

    /**
     * one iteration of checking timeout
     * @param now current time in microsecond
     */
    void timeoutIteration(int64_t now);

    void closeComponents();

protected:
    /**
     * 把[upd|tcp]:ip:port分开放在args中
     *
     * @param src: 源格式
     * @param args: 目标数组
     * @param   cnt: 数组中最大个数
     * @return  返回的数组中个数
     */
    int parseAddr(char *src, char **args, int cnt);

    /**
     * Socket event checking.
     */
    void eventLoop(SocketEvent *socketEvent);

    /**
     * Time out checking.
     */
    void timeoutLoop();
    
    /**
     * process Command in queue _commands;
     */ 
    void processCommands();

private:
    ThreadMutex _mutex;
    ThreadMutex _stopMutex;
    EPollSocketEvent _socketEvent;   
    Thread _readWriteThread;      
    Thread _timeoutThread;   
    bool _stop;              // stopping flag
    bool _started;
    bool _promotePriority;
    int64_t _nextCheckTime; 

    IOComponent *_iocListHead, *_iocListTail;   // IOComponent集合
    std::vector<TransportCommand> _commands;    // command list
    std::list<IOComponent*> _checkingList;      // timeout checking list
    std::vector<IOComponent*> _newCheckingList; // new timeout checking list
};
}

#endif /*TRANSPORT_H_*/
