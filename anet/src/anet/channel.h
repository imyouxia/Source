#ifndef ANET_CHANNEL_H_
#define ANET_CHANNEL_H_
#include <stdint.h>
namespace anet {
  class IPacketHandler;

  class Channel {
    friend class ChannelPool;
    friend class CHANNELTF; //add by hua.huangrh
    friend class CHANNELPOOLTF; //add by hua.huangrh

  public:
    /*
     * 构造函数
     */
    Channel();

    /*
     * 设置
     *
     * @param   chid    ChannelID
     */
    void setId(uint32_t id);

    /*
     * 得到Channel ID
     */
    uint32_t getId();

    /*
     * 设置回传参数
     */
    void setArgs(void *args);

    /*
     * 取到回传参数
     */
    void *getArgs();

    /*
     * 设置packethandler的句柄
     */
    void setHandler(IPacketHandler *handler);

    /*
     * 得到句柄
     */
    IPacketHandler *getHandler();

    /*
     * 设置过期时间, 绝对时间
     *
     * @param   expireTime
     */
    void setExpireTime(int64_t expireTime);

    /*
     * 下一个
     */
    Channel *getNext() {
      return _next;
    }

  private:
    uint32_t _id;      // channel id
    void *_args;    // 回传参数
    IPacketHandler *_handler;
    int64_t _expireTime; // 到期时间

  private:
    Channel *_prev;     // 用在链表
    Channel *_next;
  };
}

#endif /*CONNECTION_H_*/
