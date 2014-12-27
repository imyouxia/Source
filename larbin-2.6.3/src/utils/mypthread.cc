// Larbin
// Sebastien Ailleret
// 07-12-01 -> 07-12-01

#include <iostream.h>
#include <stdlib.h>

#include "utils/mypthread.h"

/* Launch a new thread
 * return 0 in case of success
 */
void startThread (StartFun run, void *arg) {
  pthread_t t;
  pthread_attr_t attr;
  if (pthread_attr_init(&attr) != 0
      || pthread_create(&t, &attr, run, arg) != 0
      || pthread_attr_destroy(&attr) != 0
      || pthread_detach(t) != 0) {
    cerr << "Unable to launch a thread\n";
    exit(1);
  }
}
