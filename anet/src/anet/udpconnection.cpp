#include <anet/udpconnection.h>
namespace anet {

/*
 * 构造函数
 */
UDPConnection::UDPConnection(Socket *socket, IPacketStreamer *streamer,
                             IServerAdapter *serverAdapter) : Connection(socket, streamer, serverAdapter) {}

/*
 * 析造函数
 */
UDPConnection::~UDPConnection() {}

/*
 * 写出数据
 *
 * @return 是否成功
 */
bool UDPConnection::writeData() {
    return true;
}

/*
 * 读入数据
 *
 * @return 读入数据
 */
bool UDPConnection::readData() {
    return true;
}

}
