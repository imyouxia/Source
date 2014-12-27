// Larbin
// Sebastien Ailleret
// 27-08-01 -> 15-11-01

#include "config.h"

#include <iostream.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "utils/hashDup.h"
#include "utils/connexion.h"

/* constructor */
hashDup::hashDup (ssize_t size, char *init, bool scratch) {
  this->size = size;
  file = init;
  table = new char[size / 8];
  if (init == NULL || scratch) {
    for (ssize_t i=0; i<size/8; i++) {
      table[i] = 0;
    }
  } else {
    int fds = open(init, O_RDONLY);
	if (fds < 0) {
	  cerr << "Cannot find " << init << ", restart from scratch\n";
      for (ssize_t i=0; i<size/8; i++) {
        table[i] = 0;
      }
    } else {
      ssize_t sr = 0;
      while (sr < size) {
        ssize_t tmp = read(fds, table+sr, size-sr);
        if (tmp <= 0) {
          cerr << "Cannot read " << init << "\n";
          exit(1);
        } else {
          sr += 8*tmp;
        }
      }
      close(fds);
    }
  }
}

/* destructor */
hashDup::~hashDup () {
  delete [] table;
}

/* set a page in the hashtable
 * return false if it was already there
 * return true if it was not (ie it is new)
 */
bool hashDup::testSet (char *doc) {
  unsigned int code = 0;
  char c;
  for (uint i=0; (c=doc[i])!=0; i++) {
    if (c>'A' && c<'z')
      code = (code*23 + c) % size;
  }
  unsigned int pos = code / 8;
  unsigned int bits = 1 << (code % 8);
  int res = table[pos] & bits;
  table[pos] |= bits;
  return !res;
}

/* save in a file */
void hashDup::save () {
  int fds = creat(file, 00600);
  if (fds >= 0) {
    ecrireBuff(fds, table, size/8);
	close(fds);
  }
}
