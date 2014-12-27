// Larbin
// Sebastien Ailleret
// 09-11-99 -> 07-12-01

/* fifo in RAM with synchronisations */

#ifndef SYNCFIFO_H
#define SYNCFIFO_H

#define std_size 100

#include "utils/mypthread.h"

template <class T>
class SyncFifo {
 protected:
  uint in, out;
  uint size;
  T **tab;
#ifdef THREAD_OUTPUT
  pthread_mutex_t lock;
  pthread_cond_t nonEmpty;
#endif

 public:
  /* Specific constructor */
  SyncFifo (uint size = std_size);

  /* Destructor */
  ~SyncFifo ();

  /* get the first object */
  T *get ();

  /* get the first object (non totally blocking)
   * return NULL if there is none
   */
  T *tryGet ();

  /* add an object in the Fifo */
  void put (T *obj);

  /* how many itmes are there inside ? */
  int getLength ();
};

template <class T>
SyncFifo<T>::SyncFifo (uint size) {
  tab = new T*[size];
  this->size = size;
  in = 0;
  out = 0;
  mypthread_mutex_init (&lock, NULL);
  mypthread_cond_init (&nonEmpty, NULL);
}

template <class T>
SyncFifo<T>::~SyncFifo () {
  delete [] tab;
  mypthread_mutex_destroy (&lock);
  mypthread_cond_destroy (&nonEmpty);
}

template <class T>
T *SyncFifo<T>::get () {
  T *tmp;
  mypthread_mutex_lock(&lock);
  mypthread_cond_wait(in == out, &nonEmpty, &lock);
  tmp = tab[out];
  out = (out + 1) % size;
  mypthread_mutex_unlock(&lock);
  return tmp;
}

template <class T>
T *SyncFifo<T>::tryGet () {
  T *tmp = NULL;
  mypthread_mutex_lock(&lock);
  if (in != out) {
	// The stack is not empty
	tmp = tab[out];
	out = (out + 1) % size;
  }
  mypthread_mutex_unlock(&lock);
  return tmp;
}

template <class T>
void SyncFifo<T>::put (T *obj) {
  mypthread_mutex_lock(&lock);
  tab[in] = obj;
  if (in == out) {
    mypthread_cond_broadcast(&nonEmpty);
  }
  in = (in + 1) % size;
  if (in == out) {
    T **tmp;
    tmp = new T*[2*size];
    for (uint i=out; i<size; i++) {
      tmp[i] = tab[i];
    }
    for (uint i=0; i<in; i++) {
      tmp[i+size] = tab[i];
    }
    in += size;
    size *= 2;
    delete [] tab;
    tab = tmp;
  }
  mypthread_mutex_unlock(&lock);
}

template <class T>
int SyncFifo<T>::getLength () {
  int tmp;
  mypthread_mutex_lock(&lock);
  tmp = (in + size - out) % size;
  mypthread_mutex_unlock(&lock);
  return tmp;
}

#endif // SYNCFIFO_H
