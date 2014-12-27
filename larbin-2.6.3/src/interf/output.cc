// Larbin
// Sebastien Ailleret
// 03-02-00 -> 10-12-01

#include <iostream.h>
#include <string.h>
#include <unistd.h>

#include "options.h"

#include "types.h"
#include "global.h"
#include "fetch/file.h"
#include "utils/text.h"
#include "utils/debug.h"
#include "interf/useroutput.h"
#include "utils/mypthread.h"


/** The fetch failed
 * @param u the URL of the doc
 * @param reason reason of the fail
 */
void fetchFail (url *u, FetchError err, bool interesting=false) {
#ifdef SPECIFICSEARCH
  if (interesting
      || (privilegedExts[0] != NULL && matchPrivExt(u->getFile()))) {
    failure(u, err);
  }
#else // not a SPECIFICSEARCH
  failure(u, err);
#endif
}

/** It's over with this file
 * report the situation ! (and make some stats)
 */
void endOfLoad (html *parser, FetchError err) {
  answers(err);
  switch (err) {
  case success:
#ifdef SPECIFICSEARCH
    if (parser->isInteresting) {
      interestingPage();
      loaded(parser);
    }
#else // not a SPECIFICSEARCH
    loaded(parser);
#endif // SPECIFICSEARCH
    break;
  default:
    fetchFail(parser->getUrl(), err, parser->isInteresting);
    break;
  }
}

#ifdef THREAD_OUTPUT
/** In this thread, end user manage the result of the crawl
 */
static void *startOutput (void *none) {
  initUserOutput();
  for (;;) {
    Connexion *conn = global::userConns->get();
    endOfLoad((html *)conn->parser, conn->err);
    conn->recycle();
    global::freeConns->put(conn);
  }
  return NULL;
}

void initOutput () {
  startThread(startOutput, NULL);
}

#else // THREAD_OUTPUT not defined

void initOutput () {
  initUserOutput();
}

#endif // THREAD_OUTPUT
