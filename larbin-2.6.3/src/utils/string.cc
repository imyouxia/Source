// Larbin
// Sebastien Ailleret
// 20-12-99 -> 10-12-01

#include <string.h>
#include <iostream.h>

#include "options.h"

#include "utils/text.h"
#include "utils/string.h"


// Constructor
LarbinString::LarbinString (uint size) {
  chaine = new char[size];
  this->size = size;
  pos = 0;
  chaine[0] = 0;
}

// Destructor
LarbinString::~LarbinString () {
  delete [] chaine;
}

// Recycle this string
void LarbinString::recycle (uint size) {
  if (this->size > size) {
	delete [] chaine;
	chaine = new char[size];
	this->size = size;
  }
  pos = 0;
  chaine[0] = 0;
}

// get the char *
char *LarbinString::getString () {
  return chaine;
}

/** give a new string (allocate a new one
 * the caller will have to delete it
 */
char *LarbinString::giveString () {
  return newString(chaine);
}

// append a char
void LarbinString::addChar (char c) {
  chaine[pos] = c;
  pos++;
  if (pos >= size) {
	char *tmp = new char[size * 2];
	memcpy(tmp, chaine, pos);
	delete [] chaine;
	chaine = tmp;
	size *= 2;
  }
  chaine[pos] = 0;
}

// append a char *
void LarbinString::addString (char *s) {
  uint len = strlen(s);
  addBuffer(s, len);
}

// append a buffer
void LarbinString::addBuffer (char *s, uint len) {
  if (size <= pos + len) {
    size *= 2;
    if (size <= pos + len) size = pos + len + 1;
	char *tmp = new char[size];
	memcpy(tmp, chaine, pos);
	delete [] chaine;
	chaine = tmp;
  }
  memcpy(chaine+pos, s, len);
  pos += len;
  chaine[pos] = 0;
}

// change a char
void LarbinString::setChar (uint i, char c) {
  chaine[i] = c;
}
