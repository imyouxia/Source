// Larbin
// Sebastien Ailleret
// 18-11-99 -> 07-03-02

/* This class contains all global variables */

#ifndef GLOBAL_H
#define GLOBAL_H

#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>

#include <adns.h>

#include "fetch/file.h"
#include "fetch/hashTable.h"
#include "utils/hashDup.h"
#include "utils/url.h"
#include "utils/Vector.h"
#include "utils/string.h"
#include "utils/PersistentFifo.h"
#include "utils/ConstantSizedFifo.h"
#include "utils/SyncFifo.h"
#include "utils/Fifo.h"
#include "fetch/site.h"
#include "fetch/checker.h"

#define addIPUrl() global::IPUrl++
#define delIPUrl() global::IPUrl--

/** This represent a connection : we have a fixed number of them
 * fetchOpen links them with servers
 * fetchPipe reads those which are linked
 */
struct Connexion {
  char state;      // what about this socket : EMPTY, CONNECTING, WRITE, OPEN
  int pos;         // What part of the request has been sent
  FetchError err;  // How did the fetch terminates
  int socket;      // number of the fds
  int timeout;  // timeout for this connexion
  LarbinString request;  // what is the http request
  file *parser;    // parser for the connexion (a robots.txt or an html file)
  char buffer[maxPageSize];
  /** Constructor */
  Connexion ();
  /** Dectructor : it is never used since we reuse connections */
  ~Connexion ();
  /** Recycle a connexion
   */
  void recycle ();
};

struct global {
  /** Constructor : see global.cc for details */
  global (int argc, char * argv[]);
  /** Destructor : never used */
  ~global ();
  /** current time : avoid to many calls to time(NULL) */
  static time_t now;
  /** List of pages allready seen (one bit per page) */
  static hashTable *seen;
#ifdef NO_DUP
  /** Hashtable for suppressing duplicates */
  static hashDup *hDuplicate;
#endif // NO_DUP
  /** URLs for the sequencer with high priority */
  static SyncFifo<url> *URLsPriority;
  static SyncFifo<url> *URLsPriorityWait;
  static uint readPriorityWait;
  /** This one has a lower priority : see fetch/sequencer.cc */
  static PersistentFifo *URLsDisk;
  static PersistentFifo *URLsDiskWait;
  static uint readWait;
  /** hashtables of the site we accessed (cache) */
  static NamedSite *namedSiteList;
  static IPSite *IPSiteList;
  /** Sites which have at least one url to fetch */
  static Fifo<IPSite> *okSites;
  /** Sites which have at least one url to fetch
   * but need a dns call
   */
  static Fifo<NamedSite> *dnsSites;
  /** Informations for the fetch
   * This array contain all the connections (empty or not)
   */
  static Connexion *connexions;
  /** Internal state of adns */
  static adns_state ads;
  /* Number of pending dns calls */
  static uint nbDnsCalls;
  /** free connection for fetchOpen : connections with state==EMPTY */
  static ConstantSizedFifo<Connexion> *freeConns;
#ifdef THREAD_OUTPUT
  /** free connection for fetchOpen : connections waiting for end user */
  static ConstantSizedFifo<Connexion> *userConns;
#endif
  /** Sum of the sizes of a fifo in Sites */
  static Interval *inter;
  /** How deep should we go inside a site */
  static int8_t depthInSite;
  /** Follow external links ? */
  static bool externalLinks;
  /** how many seconds should we wait beetween 2 calls at the same server 
   * 0 if you are only on a personnal server, >=30 otherwise
   */
  static time_t waitDuration;
  /** Name of the bot */
  static char *userAgent;
  /** Name of the man who lauch the bot */
  static char *sender;
  /** http headers to send with requests 
   * sends name of the robots, from field...
   */
  static char *headers;
  static char *headersRobots;  // used when asking a robots.txt
  /* internet address of the proxy (if any) */
  static sockaddr_in *proxyAddr;
  /** connect to this server through a proxy using connection conn 
   * return >0 in case of success (connecting or connected), 0 otherwise
   */
  static char getProxyFds (Connexion *conn);
  /** Limit to domain */
  static Vector<char> *domains;
  /** forbidden extensions
   * extensions which are allways to avoid : .ps, .pdf...
   */
  static Vector<char> forbExt;
  /** number of parallel connexions
   * your kernel must support a little more than nb_conn file descriptors
   */
  static uint nb_conn;
  /** number of parallel dns calls */
  static uint dnsConn;
  /** number of urls in IPSites */
  static int IPUrl;
  /** port on which is launched the http statistic webserver */
  static unsigned short int httpPort;
  /** port on which input wait for queries */
  static unsigned short int inputPort;
  /** parse configuration file */
  static void parseFile (char *file);
  /** read the domain limit */
  static void manageDomain (char **posParse);
  /** read the forbidden extensions */
  static void manageExt (char **posParse);
  /////////// POLL ///////////////////////////////////
  /** array used by poll */
  static struct pollfd *pollfds;
  /** pos of the max used field in pollfds */
  static uint posPoll;
  /** size of pollfds */
  static uint sizePoll;
  /** array used for dealing with answers */
  static short *ansPoll;
  /** number of the biggest file descriptor */
  static int maxFds;
  /** make sure the new socket is not too big for ansPoll */
  static void verifMax (int fd);
#ifdef MAXBANDWIDTH
  /** number of bits still allowed during this second */
  static long int remainBand;
#endif // MAXBANDWIDTH
};

/** set this fds for next poll */
#define setPoll(fds, event) \
  global::pollfds[global::posPoll].fd = fds; \
  global::pollfds[global::posPoll].events = event; \
  global::posPoll++

#endif // GLOBAL_H
