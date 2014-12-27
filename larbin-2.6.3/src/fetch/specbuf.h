// Larbin
// Sebastien Ailleret
// 09-12-01 -> 09-12-01

/* This is a really bad hack, but i have not found a better way
 * for doing this...
 *
 * This file is included in the definition of class html
 * You can put inside definitions you need for specificBuf
 */


#ifdef SAVE_SPECIFIC
#include "fetch/savespecbuf.h"

#elif defined(DYNAMIC_SPECIFIC)
#include "fetch/dynamicspecbuf.h"

#else // DEFAULT_SPECIFIC
// nothing special

#endif
