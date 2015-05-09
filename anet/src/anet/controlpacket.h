#ifndef ANET_CONTROL_PACKET_H_
#define ANET_CONTROL_PACKET_H_
#include <anet/packet.h>

namespace anet {

class ControlPacket : public Packet {
public:
    enum {
        CMD_BAD_PACKET = 1,
        CMD_TIMEOUT_PACKET,
	CMD_CONNECTION_CLOSED,
        CMD_END
    };

    static ControlPacket BadPacket;
    static ControlPacket TimeoutPacket;
    static ControlPacket ConnectionClosedPacket;

public:
    /**
     * @param c command code
     * @param freeDelete a flag indicating if this object will be delete when
     * free() is called by user.
     */
    ControlPacket(int c, bool freeDelete = true);
    /**
     * 是否数据包
     */
    bool isRegularPacket() {
        return false;
    }

    void free();
    /**
     * 计算出数据包的长度
     */
    void countDataLen() {}

    /**
     * 组装
     */
    bool encode(DataBuffer *output) {
        return false;
    }

    /**
     * 解开
     */
    bool decode(DataBuffer *input, PacketHeader *header) {
        return false;
    }

    /**
     * 得到类型
     */
    int getCommand() {
        return _command;
    }

    /**
     * get description of this packet
     **/
    const char* what() {
        switch(_command){
        case CMD_BAD_PACKET:
            return "Bad Packet received";
            break;
        case CMD_TIMEOUT_PACKET:
            return "Packet Timeout";
            break;
        case CMD_CONNECTION_CLOSED:
            return "Connection closing";
            break;
        default:
            return "NULL";
        }
    }
private:
    int _command;
    bool _freeDelete;
};

}

#endif /*PACKET_H_*/
