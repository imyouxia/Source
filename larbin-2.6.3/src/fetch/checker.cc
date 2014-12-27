// Larbin
// Sebastien Ailleret
// 15-11-99 -> 09-12-01

/* This modules is a filter
 * given some normalized URLs, it makes sure their extensions are OK
 * and send them if it didn't see them before
 */

#include <iostream.h>
#include <string.h>

#include "options.h"

#include "types.h"
#include "global.h"
#include "utils/url.h"
#include "utils/text.h"
#include "utils/Vector.h"
#include "fetch/hashTable.h"
#include "fetch/file.h"

#include "utils/debug.h"

/** check if an url is allready known
 * if not send it
 * @param u the url to check
 */
void check (url *u) {
  if (global::seen->testSet(u)) {
	hashUrls();  // stat
	// where should this link go ?
#ifdef SPECIFICSEARCH
	if (privilegedExts[0] != NULL
        && matchPrivExt(u->getFile())) {
	  interestingExtension();
	  global::URLsPriority->put(u);
	} else {
	  global::URLsDisk->put(u);
	}
#else // not a SPECIFICSEARCH
    global::URLsDisk->put(u);
#endif
  } else {
	// This url has already been seen
    answers(urlDup);
	delete u;
  }
}

/** Check the extension of an url
 * @return true if it might be interesting, false otherwise
 */
bool filter1 (char *host, char *file) {
  int i=0;
  if (global::domains != NULL) {
	bool ok = false;
	while ((*global::domains)[i] != NULL) {
	  ok = ok || endWith((*global::domains)[i], host);
	  i++;
	}
	if (!ok) {
	  return false;
	}
  }
  i=0;
  int l = strlen(file);
  if (endWithIgnoreCase("html", file, l)
      || file[l-1] == '/'
      || endWithIgnoreCase("htm", file, l)) {
    return true;
  }
  while (global::forbExt[i] != NULL) {
	if (endWithIgnoreCase(global::forbExt[i], file, l)) {
	  return false;
	}
	i++;
  }
  return true;
}
