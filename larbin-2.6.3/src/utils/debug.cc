// Larbin
// Sebastien Ailleret
// 15-11-99 -> 03-04-01

#include <iostream.h>

#include "options.h"

#include "types.h"

#ifndef NDEBUG

uint stateMain;          // where is the main loop
uint debug = 0;

uint sites = 0;          // Number of occupied sites
uint ipsites = 0;        // Number of occupied ip sites
uint debUrl = 0;         // How many urls in ram
uint namedUrl = 0;
uint missUrl = 0;        // Number of urls reput in ram
uint debPars = 0;        // How many parsers are there

unsigned long byte_read = 0;
unsigned long byte_write = 0;

unsigned long readRate = 0;
unsigned long readPrev = 0;
unsigned long writeRate = 0;
unsigned long writePrev = 0;

#endif // NDEBUG

#ifndef NOSTATS

uint siteSeen = 0;
uint siteDNS = 0;         // has a DNS entry
uint siteRobots = 0;
uint robotsOK = 0;

uint hashUrls = 0;
uint urls = 0;
uint pages = 0;
uint interestingPage = 0;
uint interestingSeen = 0;
uint interestingExtension = 0;
uint extensionTreated = 0;
uint answers[nbAnswers] = {0,0,0,0,0,0,0,0,0};

// variables for rates
uint urlsRate = 0;
uint urlsPrev = 0;
uint pagesRate = 0;
uint pagesPrev = 0;
uint successRate = 0;
uint successPrev = 0;
uint siteSeenRate = 0;
uint siteSeenPrev = 0;
uint siteDNSRate = 0;
uint siteDNSPrev = 0;

#endif // NOSTATS
