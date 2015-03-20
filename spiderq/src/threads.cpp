#include "threads.h"
#include "spider.h"
#include "confparser.h"

/* the number of current running thread */
int g_cur_thread_num = 0;

/* lock for changing g_cur_thread_num's value */
pthread_mutex_t gctn_lock = PTHREAD_MUTEX_INITIALIZER;

// 设置线程可分离
// http://blog.csdn.net/lhf_tiger/article/details/8291984
int create_thread(void *(*start_func)(void *), void * arg, pthread_t *pid, pthread_attr_t * pattr)
{
    pthread_attr_t attr;
    pthread_t pt;

    if (pattr == NULL) {
        pattr = &attr;
        pthread_attr_init(pattr);
        pthread_attr_setstacksize(pattr, 1024*1024);
        pthread_attr_setdetachstate(pattr, PTHREAD_CREATE_DETACHED);
    }

    if (pid == NULL)
        pid = &pt;

    int rv = pthread_create(pid, pattr, start_func, arg);
    pthread_attr_destroy(pattr);
    return rv;
}

void begin_thread()
{
    SPIDER_LOG(SPIDER_LEVEL_DEBUG, "Begin Thread %lu", pthread_self());
}

void end_thread()
{
    pthread_mutex_lock(&gctn_lock);	
    int left = g_conf->max_job_num - (--g_cur_thread_num);
    if (left == 1) {
        /* can start one thread */
        attach_epoll_task();
    } else if (left > 1) {
        /* can start two thread */
        attach_epoll_task();
        attach_epoll_task();
    } else {
        /* have reached g_conf->max_job_num , do nothing */
    }
    SPIDER_LOG(SPIDER_LEVEL_DEBUG, "End Thread %lu, cur_thread_num=%d", pthread_self(), g_cur_thread_num);
    pthread_mutex_unlock(&gctn_lock);	
}
