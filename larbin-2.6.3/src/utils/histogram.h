// Larbin
// Laurent Viennot
// 10-10-01 -> 10-10-01

// modified by Sebastien Ailleret
// 19-10-01 -> 19-10-01

/* histogram of number of pages retrieved for graphical stats */

#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "options.h"

#ifdef GRAPH

/* call this every sec to report downloads */
void histoHit (uint p, uint s);

/* call this in webserver for printing */
void histoWrite (int fds);

#else // GRAPH NOT DEFINED

#define histoHit(p,s) ((void) 0)
#define histoWrite(f) ((void) 0)

#endif // GRAPH

#endif // HISTOGRAM_H
