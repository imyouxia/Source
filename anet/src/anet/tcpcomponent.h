#ifndef ANET_TCPCOMPONENT_H_
#define ANET_TCPCOMPONENT_H_
#include <anet/iocomponent.h>

namespace anet {
  class Socket;
  class TCPComponent : public IOComponent {
    friend class TCPCOMPONENTTF;
    friend class TCPCONNECTIONTF;
  public:
    TCPComponent(Transport *owner, Socket *socket);
    ~TCPComponent();

    bool init(bool isServer = false);

    Connection *createConnection(IPacketStreamer *streamer,
                                 IServerAdapter *adapter = NULL);

    /*
     * 关闭
     */
    void close();
    void closeConnection(Connection *conn);
//     void closeSocket();
//     void closeSocketNoLock();

    /*
     * 当有数据可写到时被Transport调用
     * 
     * @return 是否成功, true - 成功, false - 失败。
     */
    bool handleWriteEvent();

    /*
     * 当有数据可读时被Transport调用
     *
     * @return 是否成功, true - 成功, false - 失败。
     */
    bool handleReadEvent();

    bool handleErrorEvent();
    /*
     * 得到connection
     * 
     * @return TCPConnection
     */
    Connection *getConnection() {
      return _connection;
    }

    /*
     * 超时检查
     * 
     * @param    now 当前时间(单位us)
     */
    bool checkTimeout(int64_t now);
    bool setState(IOCState state);
  protected:
    /*
     * 连接到socket
     */
    bool socketConnect();

    Connection *_connection;
    int64_t _startConnectTime;
  };
}

#endif /*TCPCOMPONENT_H_*/
