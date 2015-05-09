/**
 * File name: defaultpacket.h
 * Author: zhangli
 * Create time: 2008-12-25 11:10:50
 * $Id$
 * 
 * Description: ***add description here***
 * 
 */

#ifndef ANET_DEFAULTPACKET_H_
#define ANET_DEFAULTPACKET_H_
#include <anet/packet.h>

namespace anet {
class DefaultPacket : public Packet
{
public:
    DefaultPacket();
    ~DefaultPacket();
    
    bool setBody(const char *, size_t);
    bool appendBody(const char *, size_t);
    const char* getBody(size_t&) const; 
    char* getBody(size_t&); 
    const char* getBody() const;
    char* getBody();

    bool encode(DataBuffer *output);
    bool decode(DataBuffer *input, PacketHeader *header); 
    
    size_t getBodyLen();    
    bool setCapacity(size_t size);
    size_t getCapacity();

protected:
    char *_body;
    size_t _bodyLength;
    size_t _capacity;

};

}/*end namespace anet*/
#endif /*ANET_DEFAULTPACKET_H_*/
