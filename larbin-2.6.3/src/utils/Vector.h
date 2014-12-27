// Larbin
// Sebastien Ailleret
// 04-02-00 -> 10-12-01

#ifndef VECTOR_HH
#define VECTOR_HH

#include "types.h"

#include <assert.h>

template <class T>
class Vector {
 private:
  /** This array contain the object */
  T **tab;
  /** Number of object in the array */
  uint pos;
  /** Size of the array */
  uint size;

 public:
  /** Constructor */
  Vector (uint size = StdVectSize);
  /** Destructor */
  ~Vector ();
  /** Re-init this vector : empty it all */
  void recycle ();
  /** add an element to this vector */
  void addElement (T *elt);
  /** give the size of the vector */
  inline uint getLength () { return pos; }
  /** give the array containing the objects */
  inline T **getTab() { return tab; }
  /** get an element of this Vector */
  T *operator [] (uint i);
};

/** Constructor
 * @param size the initial capacity of the Vector
 */
template <class T>
Vector<T>::Vector (uint size) {
  this->size = size;
  pos = 0;
  tab = new T*[size];
}

/** Destructor */
template <class T>
Vector<T>::~Vector () {
  for (uint i=0; i<pos; i++) {
	delete tab[i];
  }
  delete [] tab;
}

/** Re-init this vector : empty it all */
template <class T>
void Vector<T>::recycle () {
  for (uint i=0; i<pos; i++) {
	delete tab[i];
  }
  pos = 0;
}

/** add an element to this vector */
template <class T>
void Vector<T>::addElement (T *elt) {
  assert (pos <= size);
  if (pos == size) {
	size *= 2;
	T **tmp = new T*[size];
	for (uint i=0; i<pos; i++) {
	  tmp[i] = tab[i];
	}
	delete [] tab;
	tab = tmp;
  }
  tab[pos] = elt;
  pos++;
}

/** get an element of this Vector */
template <class T>
T *Vector<T>::operator [] (uint i) {
  if (i<pos) {
	return tab[i];
  } else {
	return NULL;
  }
}

#endif // VECTOR_HH
