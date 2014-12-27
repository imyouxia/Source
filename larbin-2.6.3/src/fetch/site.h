// Larbin
// Sebastien Ailleret
// 08-02-00 -> 04-01-02

// This is the new structure of a site
// It includes a fifo of waiting urls

#ifndef SITE_H
#define SITE_H

#include <time.h>
#include <adns.h> 

#include "types.h"
#include "utils/Fifo.h"
#include "utils/url.h"

void initSite ();

// define for the state of a connection
enum ConnState {
  emptyC,
  connectingC,
  writeC,
  openC
};

// different state of dnsQuery
enum DnsState {
  waitDns,
  doneDns,
  errorDns,
  noConnDns
};

/** This class is intended to make sure the sum of the
 * sizes of the fifo included in the different sites
 * are not too big
 */
class Interval {
  private:
  /** Position in the interval */
  uint pos;
  /** Size of the interval */
  uint size;
 public:
  /** Constructor */
  Interval (uint size) { this->size = size; pos = 0; }
  /** Destructor */
  ~Interval () {}
  /** How many urls can we put
   * answer 0 if no urls can be put
   */
  inline uint putAll () { int res=size-pos; pos=size; return res; }
  /** Warn an url has been retrieved */
  inline void getOne () { pos--; }
  /** only for debugging, handle with care */
  inline uint getPos () { return pos; }
};

class NamedSite {
 private:
  /* string used for following CNAME chains (just one jump) */
  char *cname;
  /** we've got a good dns answer
   * get the robots.txt */
  void dnsOK ();
  /** Cannot get the inet addr
   * dnsState must have been set properly before the call */
  void dnsErr ();
  /** Delete the old identity of the site */
  void newId ();
  /** put this url in its IPSite */
  void transfer (url *u);
  /** forget this url for this reason */
  void forgetUrl (url *u, FetchError reason);
 public:
  /** Constructor */
  NamedSite ();
  /** Destructor : never used */
  ~NamedSite ();
  /* name of the site */
  char name[maxSiteSize];
  /* port of the site */
  uint16_t port;
  /* numbers of urls in ram for this site */
  uint16_t nburls;
  /* fifo of urls waiting to be fetched */
  url *fifo[maxUrlsBySite];
  uint8_t inFifo;
  uint8_t outFifo;
  void putInFifo(url *u);
  url *getInFifo();
  short fifoLength();
  /** Is this Site in a dnsSites */
  bool isInFifo;
  /** internet addr of this server */
  char dnsState;
  struct in_addr addr;
  uint ipHash;
  /* Date of expiration of dns call and robots.txt fetch */
  time_t dnsTimeout;
  /** test if a file can be fetched thanks to the robots.txt */
  bool testRobots(char *file);
  /* forbidden paths : given by robots.txt */
  Vector<char> forbidden;
  /** Put an url in the fifo
   * If there are too much, put it back in UrlsInternal
   * Never fill totally the fifo => call at least with 1 */
  void putGenericUrl(url *u, int limit, bool prio);
  inline void putUrl (url *u) { putGenericUrl(u, 15, false); }
  inline void putUrlWait (url *u) { putGenericUrl(u, 10, false); }
  inline void putPriorityUrl (url *u) { putGenericUrl(u, 5, true); }
  inline void putPriorityUrlWait (url *u) { putGenericUrl(u, 1, true); }
  /** Init a new dns query */
  void newQuery ();
  /** The dns query ended with success */
  void dnsAns (adns_answer *ans);
  /** we got the robots.txt, transfer what must be in IPSites */
  void robotsResult (FetchError res);
};

class IPSite {
 private:
  /* date of last access : avoid rapid fire */
  time_t lastAccess;
  /** Is this Site in a okSites (eg have something to fetch) */
  bool isInFifo;
  /** Get an url from the fifo
   * resize tab if too big
   */
  url *getUrl ();
 public:
  /** Constructor */
  IPSite ();
  /** Destructor : never used */
  ~IPSite ();
  /** Urls waiting for being fetched */
  Fifo<url> tab;
  /** Put an url in the fifo */
  void putUrl (url *u);
  /** fetch the fist page in the fifo okSites
   * expects at least one element in freeConns
   * return expected time for next call (0 means now)
   */
  int fetch ();
};

#endif // SITE_H
