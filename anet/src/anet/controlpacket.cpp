#include <anet/thread.h>
#include <anet/connection.h>

namespace anet {
ControlPacket ControlPacket::BadPacket(CMD_BAD_PACKET, false);
ControlPacket ControlPacket::TimeoutPacket(CMD_TIMEOUT_PACKET, false);
ControlPacket ControlPacket::ConnectionClosedPacket(CMD_CONNECTION_CLOSED, false);

ControlPacket::ControlPacket(int c, bool freeDelete) {
    _command = c;
    _freeDelete = freeDelete;
}

void  ControlPacket::free() {
    if (_freeDelete) {
        delete this;
    }
}

}
