#ifndef ANET_TCPACCEPTOR_H_
#define ANET_TCPACCEPTOR_H_
#include <anet/iocomponent.h>

namespace anet {
class IOComponent;
class TCPAcceptor : public IOComponent {

public:
    /**
     * @param owner: pointer of a transport object
     * @param socket pointer of a socket object
     * @param timeout: set timeout in millisecond for server sending replies
     */
    TCPAcceptor(Transport *owner, Socket *socket, IPacketStreamer *streamer,
		IServerAdapter *serverAdapter, int timeout = 5000 );

    bool init(bool isServer = false);

    void close();

    /***
    * 当有数据可读时被Transport调用
    * 
    * @return 是否成功, true - 成功, false - 失败。
    */
    bool handleReadEvent();

    /***
     * 在accept中没有写事件
     */
    bool handleWriteEvent() {
        return true;
    }

    bool handleErrorEvent(); 

    /**
     * 超时检查
     * 
     * @param    now 当前时间(单位us)
     */
    bool checkTimeout(int64_t now);

private:
    IPacketStreamer *_streamer;      // 数据包解析器
    IServerAdapter  *_serverAdapter; // 服务器适配器
    int _timeout;
};
}

#endif /*TCPACCEPTOR_H_*/
