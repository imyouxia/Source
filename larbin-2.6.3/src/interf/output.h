// Larbin
// Sebastien Ailleret
// 03-02-00 -> 20-11-01

#ifndef OUTPUT_H
#define OUTPUT_H

#include "global.h"

/** The fetch failed
 * @param u the URL of the doc
 * @param reason reason of the fail
 */
void fetchFail (url *u, FetchError reason, bool interesting=false);

/** It's over with this file
 * report the situation ! (and make some stats)
 */
void endOfLoad (html *parser, FetchError err);

/** Initialise output */
void initOutput ();

#endif // OUTPUT_H
