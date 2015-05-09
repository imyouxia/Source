#ifndef ANET_RUNNABLE_H_
#define ANET_RUNNABLE_H_

namespace anet {
  class Thread;
  class Runnable {

  public:
    /*
     * 析构
     */
    virtual ~Runnable() {}
    /*
     * 运行入口函数
     */
    virtual void run(Thread *thread, void *arg) = 0;
  };
}

#endif /*RUNNABLE_H_*/
