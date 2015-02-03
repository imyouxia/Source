#ifndef QTHREADS_H
#define QTHREADS_H

#include <pthread.h>

extern int create_thread(void *(*start_routine) (void *), void *arg, pthread_t * thread, pthread_attr_t * pAttr);
extern void begin_thread();
extern void end_thread();

#endif
