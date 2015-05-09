#ifndef ANET_TCPCONNECTION_H_
#define ANET_TCPCONNECTION_H_
#include <anet/databuffer.h>
#include <anet/connection.h>
namespace anet {

  class DataBuffer;
  class Socket;
  class IPacketStreamer;
  class IServerAdapter;

  class TCPConnection : public Connection {
    friend class TCPCONNECTIONTF;

  public:

    TCPConnection(Socket *socket, IPacketStreamer *streamer, IServerAdapter *serverAdapter);

    ~TCPConnection();

    /*
     * 写出数据
     *
     * @return 是否成功
     */
    bool writeData();

    /*
     * 读入数据
     *
     * @return 读入数据
     */
    bool readData();

    /*
     * 设置写完是否主动关闭
     */
    void setWriteFinishClose(bool v) {
      _writeFinishClose = v;
    }

    /*
     * 清空output的buffer
     */
    void clearOutputBuffer() {
      _output.clear();
    }

    /*
     * 清空output的buffer
     */
    void clearInputBuffer() {
      _input.clear();
    }

  private:
    DataBuffer _output;     	// 输出的buffer
    DataBuffer _input;      	// 读入的buffer
    PacketHeader _packetHeader; // 读入的packet header
    bool _gotHeader;            // packet header已经取过
    bool _writeFinishClose;     // 写完断开
};

}

#endif /*TCPCONNECTION_H_*/
