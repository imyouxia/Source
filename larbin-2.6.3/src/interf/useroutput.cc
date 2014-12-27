// Larbin
// Sebastien Ailleret
// 07-12-01 -> 10-12-01

/* This is the file you should change if you want to
 * use the data fetched by larbin.
 *
 * See useroutput.h for the interface
 *
 * See the files XXXuserouput.cc for examples */

#include "options.h"

#ifdef SIMPLE_SAVE
#include "interf/saveuseroutput.cc"

#elif defined(MIRROR_SAVE)
#include "interf/mirrorsaveuseroutput.cc"

#elif defined(STATS_OUTPUT)
#include "interf/statsuseroutput.cc"

#else // DEFAULT_OUTPUT
#include "interf/defaultuseroutput.cc"

#endif
