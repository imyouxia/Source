#ifndef ANET_ISERVERADAPTER_H
#define ANET_ISERVERADAPTER_H
#include <anet/connection.h>
#include <anet/packet.h>
#include <anet/ipackethandler.h>
namespace anet {

class IServerAdapter {
public:

    /**
     * This function is used in Server. When ANET receive request,
     * it will call handlePacket() to deal with request packet.
     * This function is Called in Connection::handlePacket().
     * In some exceptions, ControlPacket(such as TimeoutPacket,
     * BadPacket) will be received. Obviously, you should consider 
     * all these exceptions in you implementation.
     * 
     * @param connection indicate where the request packet comes
     * from, you can use this connection to send a response.
     * @param packet request packet or ControlPacket
     * @return not used temporary.
     */
    virtual IPacketHandler::HPRetCode 
      handlePacket(Connection *connection, Packet *packet) = 0;
};
}

#endif /*ISERVERADAPTER_H*/
