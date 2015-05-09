#include <anet/anet.h>
#ifndef ANET_MALICIOUS_STREAMER_H_
#define ANET_MALICIOUS_STREAMER_H_

namespace anet  {
  class MaliciousStreamer : public DefaultPacketStreamer {
  public:
  MaliciousStreamer(IPacketFactory *factory) :
    DefaultPacketStreamer(factory) , _maliciousLen(0x7fffffff){}
    bool encode(Packet *packet, DataBuffer *output);
  public:
    int _maliciousLen;
  };

}//end of namespace anet
#endif //ANET_MALICIOUS_STREAMER_H_
