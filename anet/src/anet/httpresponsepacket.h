#ifndef ANET_HTTP_RESPONSE_PACKET_H
#define ANET_HTTP_RESPONSE_PACKET_H
#include <anet/packet.h>
#include <string>
#include <ext/hash_map>
namespace anet {

struct str_hash {
    size_t operator()(const std::string& str) const {
        return __gnu_cxx::__stl_hash_string(str.c_str());
    }
};
typedef __gnu_cxx::hash_map<std::string, std::string, str_hash> STRING_MAP;
typedef STRING_MAP::iterator STRING_MAP_ITER;

#define ANET_HTTP_STATUS_OK "HTTP/1.1 200 OK\r\n"
#define ANET_HTTP_STATUS_NOTFOUND "HTTP/1.1 404 Not Found\r\n"
#define ANET_HTTP_KEEP_ALIVE "Connection: Keep-Alive\r\nKeep-Alive: timeout=10, max=10\r\n"
#define ANET_HTTP_CONN_CLOSE "Connection: close\r\n"
#define ANET_HTTP_CONTENT_TYPE "Content-Type: text/html\r\n"
#define ANET_HTTP_CONTENT_LENGTH "Content-Length: %d\r\n"

class HttpResponsePacket : public Packet {
public:
    /*
     * 构造函数
     */
    HttpResponsePacket();

    /*
     * 析构函数
     */
    ~HttpResponsePacket();

    /*
     * 计算出数据包的长度
     */
    void countDataLen();

    /*
     * 组装
     */
    bool encode(DataBuffer *output);

    /*
     * 解开
     */
    bool decode(DataBuffer *input, PacketHeader *header);

    /*
     * 设置header
     */
    void setHeader(const char *name, const char *value);

    /*
     * 设置状态
     */
    void setStatus(bool status);

    /*
     * 设置内容
     */
    void setBody(const char *body, int len);

    /*
     * 是否keepalive
     */
    void setKeepAlive(bool keepAlive);

private:
    bool _status;                   // 返回的状态, true => 200, false => 404
    char *_body;                    // 返回的内容
    int _bodyLen;                   // 返回内容找长度
    STRING_MAP _headerMap;          // 返回其他头信息
    bool _isKeepAlive;              // 是否keepalive
};

}

#endif

