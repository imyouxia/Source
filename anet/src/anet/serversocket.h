#ifndef ANET_SERVERSOCKET_H_
#define ANET_SERVERSOCKET_H_
#include <anet/socket.h>
namespace anet {

class ServerSocket : public Socket {

public:
    /*
     * 构造函数
     */
    ServerSocket();
    /*
     * accept一个新的连接
     * 
     * @return 一个Socket
     */
    Socket *accept();

    /*
     * 打开监听
     * 
     * @return 是否成功
     */
    bool listen();

private:
    int _backLog; // backlog
};

}

#endif /*SERVERSOCKET_H_*/
