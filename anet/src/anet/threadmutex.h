#ifndef ANET_MUTEX_H_
#define ANET_MUTEX_H_

#include <pthread.h>
#include <assert.h>

namespace anet {

/*
 * author cjxrobot
 *
 * Linux线程锁
 */

class ThreadMutex {

public:
    /*
     * 构造函数
     */
    ThreadMutex() {
        int ret = pthread_mutex_init(&_mutex, NULL);
        assert(ret == 0);
    }

    /*
     * 析造函数
     */
    ~ThreadMutex() {
        pthread_mutex_destroy(&_mutex);
    }

    /*
     * 加锁
     */

    void lock () {
        pthread_mutex_lock(&_mutex);
    }

    /*
     * 解锁
     */
    void unlock() {
        pthread_mutex_unlock(&_mutex);
    }

protected:

    pthread_mutex_t _mutex;
};


class MutexGuard {
public:
    MutexGuard(ThreadMutex * mutex) {
        _mutex=mutex;
        if (_mutex) {
            _mutex->lock();
        }
    }
    ~MutexGuard(){
        if (_mutex) {
            _mutex->unlock();
        }
    }
private:
    ThreadMutex *_mutex;
};

}
#endif /*MUTEX_H_*/
