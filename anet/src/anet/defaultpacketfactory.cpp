/**
 * File name: defaultpacketfactory.cpp
 * Author: zhangli
 * Create time: 2008-12-25 11:38:19
 * $Id$
 * 
 * Description: ***add description here***
 * 
 */

#include <anet/defaultpacketfactory.h>
#include <anet/defaultpacket.h>
#include <anet/packet.h>

namespace anet {
Packet *DefaultPacketFactory::createPacket(int pcode) {
    return new DefaultPacket;
}

}/*end namespace anet*/
