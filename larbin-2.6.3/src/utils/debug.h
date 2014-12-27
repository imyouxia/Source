// Larbin
// Sebastien Ailleret
// 15-11-99 -> 08-05-01

// It's dirty, but I don't care
// it's just for debugging purposes

#ifndef DEBUG_H
#define DEBUG_H

#include "options.h"

////////////////////////////////////////////////
// debug
////////////////////////////////////////////////

#ifndef NDEBUG

// Where are the different threads
extern uint stateMain;
extern uint debug;

// This can be usefull when having very big problem !!!
/* #define stateMain(i) (cerr << "stateMain " << i << "\n") */

#define stateMain(i) (stateMain = i)
#define incDebug() debug++;
#define debug(i) (debug = i)

// Debug new and delete
extern uint sites;
extern uint ipsites;
extern uint debUrl;    // number of urls
extern uint namedUrl;  // urls in namedSites
extern uint missUrl;
extern uint debPars;

#define addsite() sites++
#define addipsite() ipsites++
#define newUrl() debUrl++
#define refUrl() missUrl++
#define delUrl() debUrl--
#define newPars() debPars++
#define delPars() debPars--

#define addNamedUrl() namedUrl++
#define delNamedUrl() namedUrl--

// number of byte read and written
extern unsigned long byte_read;
extern unsigned long byte_write;

#define addRead(i) (byte_read += i)
#define addWrite(i) (byte_write += i)

extern unsigned long readRate;
extern unsigned long readPrev;
extern unsigned long writeRate;
extern unsigned long writePrev;

#else // NDEBUG

#define stateMain(i) ((void) 0)
#define incDebug() ((void) 0)
#define debug(i) ((void) 0)

#define addsite() ((void) 0)
#define addipsite() ((void) 0)
#define newUrl() ((void) 0)
#define delUrl() ((void) 0)
#define newPars() ((void) 0)
#define refUrl() ((void) 0)
#define delPars() ((void) 0)

#define addNamedUrl() ((void) 0)
#define delNamedUrl() ((void) 0)

#define addRead(i) ((void) 0)
#define addWrite(i) ((void) 0)

#endif // NDEBUG

////////////////////////////////////////////////
// stats
////////////////////////////////////////////////

#ifndef NOSTATS

extern uint siteSeen;
extern uint siteDNS;  // has a DNS entry
extern uint siteRobots;
extern uint robotsOK;
#define siteSeen() siteSeen++
#define siteDNS() siteDNS++
#define siteRobots() siteRobots++
#define robotsOK() robotsOK++
#define robotsOKdec() robotsOK--

extern uint hashUrls;
extern uint urls;
extern uint pages;
extern uint interestingPage;
extern uint interestingSeen;
extern uint interestingSuccess;
extern uint interestingExtension;
extern uint extensionTreated;
extern uint answers[nbAnswers];
#define hashUrls() hashUrls++;
#define urls() urls++
#define pages() pages++
#define interestingPage() interestingPage++
#define interestingSeen() interestingSeen++
#define interestingSuccess() interestingSuccess++
#define interestingExtension() interestingExtension++
#define extensionTreated() extensionTreated++
#define answers(i) answers[i]++

// variables for rates
extern uint urlsRate;
extern uint urlsPrev;
extern uint pagesRate;
extern uint pagesPrev;
extern uint successRate;
extern uint successPrev;
extern uint siteSeenRate;
extern uint siteSeenPrev;
extern uint siteDNSRate;
extern uint siteDNSPrev;

#else // NOSTATS

#define siteSeen() ((void) 0)
#define siteDNS() ((void) 0)
#define siteRobots() ((void) 0)
#define robotsOK() ((void) 0)
#define robotsOKdec() ((void) 0)

#define hashUrls() ((void) 0)
#define urls() ((void) 0)
#define pages() ((void) 0)
#define interestingPage() ((void) 0)
#define interestingSeen() ((void) 0)
#define interestingExtension() ((void) 0)
#define extensionTreated() ((void) 0)
#define answers(i) ((void) 0)

#endif // NOSTATS

#ifdef CRASH
#define crash(s) (cerr << s << "\n")
#else // CRASH
#define crash(s) ((void) 0)
#endif // CRASH

#endif // DEBUG_H
