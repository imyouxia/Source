#ifndef ANET_SOCKET_H_
#define ANET_SOCKET_H_
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>

namespace anet {
  class IOComponent;
  class ThreadMutex;
class Socket {

public:
    /*
     * 构造函数
     */
    Socket();

    /*
     * 析构函数
     */
    ~Socket();

    /*
     * 设置地址
     *
     * @param address  host或ip地址
     * @param port  端口号
     * @return 是否成功
     */

    bool setAddress (const char *address, const int port);

    /*
     * 连接到_address上
     * 
     * @return 是否成功
     */
    bool connect();

    /*
     * 重连连接到_address上
     *
     * @return 是否成功
     */
    bool reconnect();    

    /**
     * 关闭连接
     */
    void close();

    /*
     * 关闭读写
     */
    void shutdown();

    /**
     * 使用UDP的socket
     * 
     * @return 是否成功
     */
    bool createUDP();

    /*
     * 把socketHandle,及ipaddress设置到此socket中
     * 
     * @param  socketHandle: socket的文件句柄
     * @param hostAddress: 服务器地址
     */

    void setUp(int socketHandle, struct sockaddr *hostAddress);

    /*
     * 返回文件句柄
     * 
     * @return 文件句柄
     */
    int getSocketHandle();

    /*
     * 返回IOComponent
     * 
     * @return  IOComponent
     */
    IOComponent *getIOComponent();

    /*
     * 设置IOComponent
     * 
     * @param IOComponent
     */
    void setIOComponent(IOComponent *ioc);

    /*
     * 写数据
     */
    int write(const void *data, int len);

    /*
     * 读数据
     */
    int read(void *data, int len);

    /*
     * SetSoKeepAlive
     */
    bool setKeepAlive(bool on) {
        return setIntOption(SO_KEEPALIVE, on ? 1 : 0);
    }

    /*
     * setReuseAddress
     */
    bool setReuseAddress(bool on) {
        return setIntOption(SO_REUSEADDR, on ? 1 : 0);
    }

    /*
     * setSoLinger
     */
    bool setSoLinger (bool doLinger, int seconds);

    /*
     * setTcpNoDelay
     */
    bool setTcpNoDelay(bool noDelay);

    /*
     * setIntOption
     */
    bool setIntOption(int option, int value);

    /*
     * 是否阻塞
     */
    bool setSoBlocking(bool on);

    /*
     * 检查Socket句柄是否创建
     */
    bool checkSocketHandle();

    /*
     * 得到Socket错误
     */
    int getSoError();

    int getPort(bool active = true);
    uint32_t getIpAddress(bool active = true);
    /*
     * 得到ip地址, 写到tmp上
     */
    char *getAddr(char *dest, int len, bool active = false);

    /*
     * 得到最后的错误
     */
    static int getLastError() {
        return errno;
    }

    inline bool isAddressValid() {
      return _addressValid;
    }

    bool getSockAddr(sockaddr_in &addr, bool active = true);
protected:
    struct sockaddr_in  _address; // 地址
    int _socketHandle;    // socket文件句柄
    IOComponent *_iocomponent;
    static ThreadMutex _dnsMutex; //　多实例用一个dnsMutex
    bool _addressValid;
    
};
}

#endif /*SOCKET_H_*/
