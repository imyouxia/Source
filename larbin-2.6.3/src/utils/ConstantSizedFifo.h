// Larbin
// Sebastien Ailleret
// 15-04-00 -> 07-12-01

/* this fifo will not grow
 * it is synchronized
 */

#ifndef CONSTANTFIFO_H
#define CONSTANTFIFO_H

#include <assert.h>

#include "utils/mypthread.h"

template <class T>
class ConstantSizedFifo {
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
  ConstantSizedFifo (uint size);
  /* Destructor */
  ~ConstantSizedFifo ();
  /* get the first object */
  T *get ();
  /* get the first object (non totally blocking)
   * return NULL if there is none
   */
  T *tryGet ();
  /* add an object in the fifo */
  void put (T *obj);
  /* add an object in the fifo
   * never block !!!
   */
  int getLength ();
  /* is there something inside ? */
  bool isNonEmpty ();
};

template <class T>
ConstantSizedFifo<T>::ConstantSizedFifo (uint size) {
  this->size = size+1;
  tab = new T*[this->size];
  in = 0;
  out = 0;
  mypthread_mutex_init (&lock, NULL);
  mypthread_cond_init (&nonEmpty, NULL);
}

template <class T>
ConstantSizedFifo<T>::~ConstantSizedFifo () {
  delete [] tab;
  mypthread_mutex_destroy (&lock);
  mypthread_cond_destroy (&nonEmpty);
}

template <class T>
T *ConstantSizedFifo<T>::get () {
  T *tmp;
  mypthread_mutex_lock(&lock);
  mypthread_cond_wait(in == out, &nonEmpty, &lock);
  tmp = tab[out];
  out = (out + 1) % size;
  mypthread_mutex_unlock(&lock);
  return tmp;
}

template <class T>
T *ConstantSizedFifo<T>::tryGet () {
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
void ConstantSizedFifo<T>::put (T *obj) {
  mypthread_mutex_lock(&lock);
  tab[in] = obj;
  if (in == out) {
    mypthread_cond_broadcast(&nonEmpty);
  }
  in = (in + 1) % size;
  assert (in != out);
  mypthread_mutex_unlock(&lock);
}

template <class T>
int ConstantSizedFifo<T>::getLength () {
  int tmp;
  mypthread_mutex_lock(&lock);
  tmp = (in + size - out) % size;
  mypthread_mutex_unlock(&lock);
  return tmp;
}

template <class T>
bool ConstantSizedFifo<T>::isNonEmpty () {
  mypthread_mutex_lock(&lock);
  bool res = (in != out);
  mypthread_mutex_unlock(&lock);
  return res;
}

#endif // CONSTANTFIFO_H
