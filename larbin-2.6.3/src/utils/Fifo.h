// Larbin
// Sebastien Ailleret
// 09-11-99 -> 01-12-01

/* standard fifo in RAM WITHOUT synchronisation */

#ifndef FIFO_H
#define FIFO_H

template <class T>
class Fifo {
 public:
  uint in, out;
  uint size;
  T **tab;

  /* Specific constructor */
  Fifo (uint size=maxUrlsBySite);

  /* Destructor */
  ~Fifo ();

  /* give the first object and let it in */
  inline T* read () { return tab[out]; }

  /* read the first obj if exist */
  T *tryRead ();

  /* get the first object */
  T *get ();

  /* get the first object (non totally blocking)
   * return NULL if there is none
   */
  T *tryGet ();

  /* add an object in the Fifo */
  void put (T *obj);

  /* put an obj that has just been get
   * this function must be called only to put back an obj
   *    that has just been tken with get */
  void rePut (T *obj);

  /* how many items are there inside ? */
  int getLength ();

  /* is this fifo empty ? */
  inline bool isEmpty () { return in == out; }
};

template <class T>
Fifo<T>::Fifo (uint size) {
  tab = new T*[size];
  this->size = size;
  in = 0;
  out = 0;
}

template <class T>
Fifo<T>::~Fifo () {
  delete [] tab;
}

template <class T>
T *Fifo<T>::tryRead () {
  if (in == out) {
    return NULL;
  } else {
    return tab[out];
  }
}

template <class T>
T *Fifo<T>::get () {
  T *tmp;
  assert (in != out);
  tmp = tab[out];
  out = (out + 1) % size;
  return tmp;
}

template <class T>
T *Fifo<T>::tryGet () {
  T *tmp = NULL;
  if (in != out) {
	// The stack is not empty
	tmp = tab[out];
	out = (out + 1) % size;
  }
  return tmp;
}

template <class T>
void Fifo<T>::put (T *obj) {
  tab[in] = obj;
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
}

template <class T>
void Fifo<T>::rePut (T *obj) {
  out = (out + size - 1) % size;
  tab[out] = obj;
}

template <class T>
int Fifo<T>::getLength () {
  return (in + size - out) % size;
}

#endif // FIFO_H
