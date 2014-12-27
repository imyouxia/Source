// Larbin
// Sebastien Ailleret
// 23-11-99 -> 15-02-01

#include <iostream.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "options.h"

#include "global.h"
#include "types.h"
#include "utils/url.h"
#include "utils/connexion.h"
#include "fetch/hashTable.h"

/* constructor */
hashTable::hashTable (bool create) {
  ssize_t total = hashSize/8;
  table = new char[total];
  if (create) {
	for (ssize_t i=0; i<hashSize/8; i++) {
	  table[i] = 0;
	}
  } else {
	int fds = open("hashtable.bak", O_RDONLY);
	if (fds < 0) {
	  cerr << "Cannot find hashtable.bak, restart from scratch\n";
      for (ssize_t i=0; i<hashSize/8; i++) {
        table[i] = 0;
      }
	} else {
      ssize_t sr = 0;
      while (sr < total) {
        ssize_t tmp = read(fds, table+sr, total-sr);
        if (tmp <= 0) {
          cerr << "Cannot read hashtable.bak : "
               << strerror(errno) << endl;
          exit(1);
        } else {
          sr += tmp;
        }
      }
      close(fds);
    }
  }
}

/* destructor */
hashTable::~hashTable () {
  delete [] table;
}

/* save the hashTable in a file */
void hashTable::save() {
  rename("hashtable.bak", "hashtable.old");
  int fds = creat("hashtable.bak", 00600);
  if (fds >= 0) {
    ecrireBuff(fds, table, hashSize/8);
	close(fds);
  }
  unlink("hashtable.old");
}

/* test if this url is allready in the hashtable
 * return true if it has been added
 * return false if it has allready been seen
 */
bool hashTable::test (url *U) {
  unsigned int code = U->hashCode();
  unsigned int pos = code / 8;
  unsigned int bits = 1 << (code % 8);
  return table[pos] & bits;
}

/* set a url as present in the hashtable
 */
void hashTable::set (url *U) {
  unsigned int code = U->hashCode();
  unsigned int pos = code / 8;
  unsigned int bits = 1 << (code % 8);
  table[pos] |= bits;
}

/* add a new url in the hashtable
 * return true if it has been added
 * return false if it has allready been seen
 */
bool hashTable::testSet (url *U) {
  unsigned int code = U->hashCode();
  unsigned int pos = code / 8;
  unsigned int bits = 1 << (code % 8);
  int res = table[pos] & bits;
  table[pos] |= bits;
  return !res;
}
