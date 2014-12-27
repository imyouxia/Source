// Larbin
// Sebastien Ailleret
// 06-01-00 -> 12-06-01

/* this fifo is stored on disk */

#ifndef PERSFIFO_H
#define PERSFIFO_H

#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "types.h"
#include "utils/url.h"
#include "utils/text.h"
#include "utils/connexion.h"
#include "utils/mypthread.h"

class PersistentFifo {
 protected:
  uint in, out;
#ifdef THREAD_OUTPUT
  pthread_mutex_t lock;
#endif
  // number of the file used for reading
  int fin, fout;
  // name of files
  uint fileNameLength;
  char *fileName;
  // Make fileName fit with this number
  void makeName (uint nb);
  // Give a file name for this int
  int getNumber (char *file);
  // Change the file used for reading
  void updateRead ();
  // Change the file used for writing
  void updateWrite ();
  // buffer used for readLine
  char outbuf[BUF_SIZE];
  // number of char used in this buffer
  uint outbufPos;
  // buffer used for readLine
  char buf[BUF_SIZE];
  // number of char used in this buffer
  uint bufPos, bufEnd;
  // sockets for reading and writing
  int rfds, wfds;
  // read a line on rfds
  char *readLine ();
  // write an url in the out file (buffered write)
  void writeUrl (char *s);
  // Flush the out Buffer in the outFile
  void flushOut ();

 public:
  /* Specific constructor */
  PersistentFifo (bool reload, char *baseName);

  /* Destructor */
  ~PersistentFifo ();

  /* get the first object (non totally blocking)
   * return NULL if there is none
   */
  url *tryGet ();

  /* get the first object (non totally blocking)
   * probably crash if there is none
   */
  url *get ();

  /* add an object in the fifo */
  void put (url *obj);

  /* how many items are there inside ? */
  int getLength ();
};

#endif // PERSFIFO_H
