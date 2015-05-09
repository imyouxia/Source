/**
 * File name: defaultpacketfactory.h
 * Author: zhangli
 * Create time: 2008-12-25 11:38:19
 * $Id$
 * 
 * Description: ***add description here***
 * 
 */

#ifndef ANET_DEFAULTPACKETFACTORY_H_
#define ANET_DEFAULTPACKETFACTORY_H_
#include <anet/ipacketfactory.h>

namespace anet {
class Packet;
class DefaultPacketFactory : public IPacketFactory
{
public:
    Packet *createPacket(int pcode);
};

}/*end namespace anet*/
#endif /*ANET_DEFAULTPACKETFACTORY_H_*/
