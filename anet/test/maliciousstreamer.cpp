#include <anet/log.h>
#include "maliciousstreamer.h"
namespace anet {
  bool MaliciousStreamer::encode(Packet *packet, DataBuffer *output) {
    ANET_LOG(DEBUG, "Malicious Encode() called");
    PacketHeader *header = packet->getPacketHeader();

    // 为了当encode失败恢复时用
    int oldLen = output->getDataLen();
    // dataLençäç
    int dataLenOffset = -1;
    int headerSize = 0;

    // 允许存在头信息,写出头信息
    if (_existPacketHeader) {
      output->writeInt32(ANET_PACKET_FLAG);
      output->writeInt32(header->_chid);
      output->writeInt32(header->_pcode);
      dataLenOffset = output->getDataLen();
      output->writeInt32(0);
      headerSize = 4 * sizeof(int32_t);
    }
    // 写数据
    if (packet->encode(output) == false) {
      ANET_LOG(ERROR, "encode error");
      output->stripData(output->getDataLen() - oldLen);
      return false;
    }
    // 计算包长度
    header->_dataLen = output->getDataLen() - oldLen - headerSize;
    // 最终把长度回到buffer中
    if (dataLenOffset >= 0) {
      unsigned char *ptr = (unsigned char *)(output->getData() + dataLenOffset);
      ANET_LOG(DEBUG, "Write a malicious length(%08x)", _maliciousLen);
      output->fillInt32(ptr, _maliciousLen);

    }
    return true;
    ANET_LOG(DEBUG, "Malicious Encode() call return");
  }

}//end of namespace anet
