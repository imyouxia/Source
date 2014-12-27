// Larbin
// Sebastien Ailleret
// 15-11-99 -> 04-01-02

#include <iostream.h>

#include "options.h"

#include "global.h"
#include "types.h"
#include "utils/url.h"
#include "utils/debug.h"
#include "fetch/site.h"

static bool canGetUrl (bool *testPriority);
uint space = 0;

#define maxPerCall 100

/** start the sequencer
 */
void sequencer () {
  bool testPriority = true;
  if (space == 0) {
    space = global::inter->putAll();
  }
  int still = space;
  if (still > maxPerCall) still = maxPerCall;
  while (still) {
    if (canGetUrl(&testPriority)) {
      space--; still--;
    } else {
      still = 0;
    }
  }
}

/* Get the next url
 * here is defined how priorities are handled
 */
static bool canGetUrl (bool *testPriority) {
  url *u;
  if (global::readPriorityWait) {
    global::readPriorityWait--;
    u = global::URLsPriorityWait->get();
    global::namedSiteList[u->hostHashCode()].putPriorityUrlWait(u);
    return true;
  } else if (*testPriority && (u=global::URLsPriority->tryGet()) != NULL) {
    // We've got one url (priority)
    global::namedSiteList[u->hostHashCode()].putPriorityUrl(u);
    return true;
  } else {
    *testPriority = false;
    // Try to get an ordinary url
    if (global::readWait) {
      global::readWait--;
      u = global::URLsDiskWait->get();
      global::namedSiteList[u->hostHashCode()].putUrlWait(u);
      return true;
    } else {
      u = global::URLsDisk->tryGet();
      if (u != NULL) {
        global::namedSiteList[u->hostHashCode()].putUrl(u);
        return true;
      } else {
        return false;
      }
    }
  }
}
