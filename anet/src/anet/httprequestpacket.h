#ifndef ANET_HTTP_REQUEST_PACKET_H
#define ANET_HTTP_REQUEST_PACKET_H
#include <anet/packet.h>
#include <ext/hash_map>
namespace anet {
class DataBuffer;
class PacketHeader;
struct eqstr {
    bool operator()(const char* s1, const char* s2) const {
        return strcmp(s1, s2) == 0;
    }
};
typedef __gnu_cxx::hash_map<const char*, const char*, __gnu_cxx::hash<const char*>, eqstr> PSTR_MAP;
typedef PSTR_MAP::iterator PSTR_MAP_ITER;

class HttpRequestPacket : public Packet {
  friend class HttpRequestAndResponsePacketTF;
public:
    /*
     * 构造函数
     */
    HttpRequestPacket();

    /*
     * 析构函数
     */
    ~HttpRequestPacket();


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
     * 查询串
     */
    char *getQuery();

    /*
     * 是否keepalive
     */
    bool isKeepAlive();

    /*
     * 寻找其他头信息
     */
    const char *findHeader(const char *name);

private:
    char *_strHeader;       // 保存头内容的buffer
    char *_strQuery;        // 查询串
    bool _isKeepAlive;      // 是否支持keepalive
    int _method;            // get - 1
    int _version;           // http version 1-"HTTP/1.0"; 2="HTTP/1.1+"
    PSTR_MAP _headerMap;    // 其他头信息的map
};

}

#endif

