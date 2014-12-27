// Larbin
// Sebastien Ailleret
// 20-12-99 -> 05-05-01

#ifndef STRING_HH
#define STRING_HH

#include <assert.h>

#include "types.h"
#include "utils/debug.h"

class LarbinString {
 private:
  char *chaine;
  uint pos;
  uint size;
 public:
  // Constructor
  LarbinString (uint size=STRING_SIZE);
  // Destructor
  ~LarbinString ();
  // Recycle this string
  void recycle (uint size=STRING_SIZE);
  // get the char * : it is deleted when you delete this String Object
  char *getString ();
  // give a char * : it creates a new one : YOU MUST DELETE IT YOURSELF
  char *giveString ();
  // append a char
  void addChar (char c);
  // append a char *
  void addString (char *s);
  // append a buffer
  void addBuffer (char *s, uint len);
  // length of this string
  inline uint getLength () { return pos; };
  // get a char of this string
  inline char operator [] (uint i) {
    assert(i<=pos);
    return chaine[i];
  }
  // change a char
  void setChar (uint i, char c);
};

#endif // STRING_HH
