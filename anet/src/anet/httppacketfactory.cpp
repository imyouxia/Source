/**
 * File name: httppacketfactory.cpp
 * Author: zhangli
 * Create time: 2008-12-19 16:36:39
 * $Id$
 * 
 * Description: ***add description here***
 * 
 */

#include <anet/httppacketfactory.h>
#include <anet/httppacket.h>

namespace anet {
HTTPPacketFactory::HTTPPacketFactory() {
}

Packet* HTTPPacketFactory::createPacket(int pcode) {
    return new HTTPPacket;
}

}/*end namespace anet*/
