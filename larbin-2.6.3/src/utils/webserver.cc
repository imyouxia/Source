// Larbin
// Sebastien Ailleret
// 07-03-00 -> 17-03-02

/* This is the code for the webserver which is used 
 * for giving stats about the program while it is running
 *
 * It is very UNefficient, but i don't care since it should
 * not be used very often. Optimize here is a waste of time */

#ifndef NOWEBSERVER

#include <string.h>
#include <unistd.h>
#include <iostream.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "options.h"

#include "global.h"
#include "fetch/sequencer.h"
#include "utils/text.h"
#include "utils/connexion.h"
#include "utils/debug.h"
#include "utils/histogram.h"
#include "interf/useroutput.h"

static char *readRequest (int fds);
static void manageAns (int fds, char *req);

static time_t startTime;
static char *startDate;

// a buffer used for various things (read request, write urls...)
static char buf[BUF_SIZE];

void *startWebserver (void *none) {
  // bind the socket
  int fds;
  int nAllowReuse = 1;
  struct sockaddr_in addr;
  startTime = global::now;
  startDate = newString(ctime(&startTime));
  memset(&addr, 0, sizeof(addr));
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(global::httpPort);
  if ((fds = socket(AF_INET, SOCK_STREAM, 0)) == -1
	  || setsockopt(fds, SOL_SOCKET, SO_REUSEADDR, (char*)&nAllowReuse, sizeof(nAllowReuse))
	  || bind(fds, (struct sockaddr *) &addr, sizeof(addr)) != 0
	  || listen(fds, 4) != 0) {
	cerr << "Unable to get the socket for the webserver (" << global::httpPort
         << ") : " << strerror(errno) << endl;
	exit(1);
  }
  // answer requests
  for (;;) {
	struct sockaddr_in addrc;
	int fdc;
	uint len = sizeof(addr);
	fdc = accept(fds, (struct sockaddr *) &addrc, &len);
	if (fdc == -1) {
	  cerr << "Trouble with web server...\n";
	} else {
      manageAns(fdc, readRequest(fdc));
      close(fdc);
	}
  }
  return NULL;
}


////////////////////////////////////////////////////////////
// write answer

static void writeHeader(int fds) {
  ecrire(fds, "HTTP/1.0 200 OK\r\nServer: Larbin\r\nContent-type: text/html\r\n\r\n<html>\n<head>\n<title>");
  ecrire(fds, global::userAgent);
  ecrire(fds, " real time statistic</title>\n</head>\n<body bgcolor=\"#FFFFFF\">\n<center><h1>Larbin is up and running !</h1></center>\n<pre>\n");
}

static void writeFooter(int fds) {
  // end of page and kill the connexion
  ecrire(fds, "\n</pre>\n<a href=\"stats.html\">stats</a>\n(<a href=\"smallstats.html\">small stats</a>+<a href=\"graph.html\">graphics</a>)\n<a href=\"debug.html\">debug</a>\n<a href=\"all.html\">all</a>\n<a href=\"ip.html\">ip</a>\n<a href=\"dns.html\">dns</a>\n<a href=\"output.html\">output</a>\n<hr>\n<A HREF=\"http://larbin.sourceforge.net/\"><img SRC=\"http://perso.wanadoo.fr/sebastien.ailleret/seb.gif\"></A>\n<A HREF=\"mailto:sebastien@ailleret.com\">sebastien@ailleret.com</A>\n</body>\n</html>");
  shutdown(fds, 2);
}

static int totalduree;

/** write date and time informations */
static void writeTime (int fds) {
  ecrire(fds, "Start date :   ");
  ecrire(fds, startDate);
  ecrire(fds, "Current date : ");
  ecrire(fds, ctime(&global::now));
  ecrire(fds, "up: ");
  int duree = global::now - startTime;
  totalduree = duree;
  if (duree > 86400) {
    ecrireInt(fds, duree / 86400);
    duree %= 86400;
    ecrire(fds, " day(s) ");
  }
  ecrireInt(fds, duree/3600);
  duree %= 3600;
  ecrire(fds, ":");
  ecrireInt2(fds, duree/60);
  duree %= 60;
  ecrire(fds, ":");
  ecrireInt2(fds, duree);
}

/* write stats information (if any) */
#ifdef NOSTATS
#define writeStats(fds) ((void) 0)
#else

// specific part
#ifdef SPECIFICSEARCH
static void writeSpecStats(int fds) {
  ecrire(fds, "\n\n<h2>Interesting pages (");
  ecrire(fds, contentTypes[0]);
  int i=1;
  while (contentTypes[i] != NULL) {
    ecrire(fds, ", ");
    ecrire(fds, contentTypes[i]);
    i++;
  }
  ecrire(fds, ") :</h2>\ntotal fetched (success)          : ");
  ecrireInti(fds, interestingPage, "%5d");
  ecrire(fds, "\ntotal fetched (error or success) : ");
  ecrireInti(fds, interestingSeen, "%5d");
  if (privilegedExts[0] != NULL) {
    ecrire(fds, "\nprivileged links seen (");
    ecrire(fds, privilegedExts[0]);
    int i=1;
    while (privilegedExts[i] != NULL) {
      ecrire(fds, ", ");
      ecrire(fds, privilegedExts[i]);
      i++;
    }
    ecrire(fds, ") :     ");
    ecrireInti(fds, interestingExtension, "%5d");
    ecrire(fds, "\nprivileged links fetched :         ");
    ecrireInti(fds, extensionTreated, "%5d");
  }
}
#else
#define writeSpecStats(fds) ((void) 0)
#endif

// main part
static void writeStats (int fds) {
  writeSpecStats(fds);
  ecrire(fds, "\n\n<h2>Pages :</h2>\n\nurls treated : ");
  ecrireInti(fds, urls, "%13d");
  ecrire(fds, "   (current rate : ");
  ecrireInti(fds, urlsRate, "%3d");
  ecrire(fds, ", overall rate : ");
  ecrireInti(fds, urls / totalduree, "%3d");
  ecrire(fds, ")\nforb robots : ");
  ecrireInti(fds, answers[forbiddenRobots], "%14d");
  ecrire(fds, "\nno dns : ");
  ecrireInti(fds, answers[noDNS], "%19d");
  ecrire(fds, "\n\nPages : ");
  ecrireInti(fds, pages, "%20d");
  ecrire(fds, "   (current rate : ");
  ecrireInti(fds, pagesRate, "%3d");
  ecrire(fds, ", overall rate : ");
  ecrireInti(fds, pages / totalduree, "%3d");
  ecrire(fds, ")\nSuccess : ");
  ecrireInti(fds, answers[success], "%18d");
  ecrire(fds, "   (current rate : ");
  ecrireInti(fds, successRate, "%3d");
  ecrire(fds, ", overall rate : ");
  ecrireInti(fds, answers[success] / totalduree, "%3d");
  ecrire(fds, ")\nno connection : ");
  ecrireInti(fds, answers[noConnection], "%12d");
  ecrire(fds, "\nearly stop : ");
  ecrireInti(fds, answers[earlyStop], "%15d");
  ecrire(fds, "\ntimeout : ");
  ecrireInti(fds, answers[timeout], "%18d");
  ecrire(fds, "\nbad type : ");
  ecrireInti(fds, answers[badType], "%17d");
  ecrire(fds, "\ntoo big : ");
  ecrireInti(fds, answers[tooBig], "%18d");
  ecrire(fds, "\nerr 30X : ");
  ecrireInti(fds, answers[err30X], "%18d");
  ecrire(fds, "\nerr 40X : ");
  ecrireInti(fds, answers[err40X], "%18d");
#ifdef NO_DUP
  ecrire(fds, "\nduplicate : ");
  ecrireInti(fds, answers[duplicate], "%16d");
#endif // NO_DUP
  ecrire(fds, "\n\nurls accepted : ");
  ecrireInt(fds, hashUrls);
  ecrire(fds, " / ");
  ecrireInt(fds, hashSize);
  ecrire(fds, "\n\nfast robots : ");
  ecrireInti(fds, answers[fastRobots], "%14d");
  ecrire(fds, "\nfast no conn : ");
  ecrireInti(fds, answers[fastNoConn], "%13d");
  ecrire(fds, "\nfast no dns : ");
  ecrireInti(fds, answers[fastNoDns], "%14d");
  ecrire(fds, "\ntoo deep : ");
  ecrireInti(fds, answers[tooDeep], "%17d");
  ecrire(fds, "\ndup url : ");
  ecrireInti(fds, answers[urlDup], "%18d");

  ecrire(fds, "\n\n<h2>Sites seen (dns call done) :</h2>\n\ntotal number : ");
  ecrireInti(fds, siteSeen, "%18d");
  ecrire(fds, " +");
  ecrireInti(fds, global::nbDnsCalls, "%2d");
  ecrire(fds, "   (current rate : ");
  ecrireInti(fds, siteSeenRate, "%2d");
  ecrire(fds, ", overall rate : ");
  ecrireInti(fds, siteSeen / totalduree, "%2d");
  ecrire(fds, ")\nwith dns : ");
  ecrireInti(fds, siteDNS, "%22d");
  ecrire(fds, "       (current rate : ");
  ecrireInti(fds, siteDNSRate, "%2d");
  ecrire(fds, ", overall rate : ");
  ecrireInti(fds, siteDNS / totalduree, "%2d");
  ecrire(fds, ")\nwith robots.txt : ");
  ecrireInti(fds, siteRobots, "%15d");
  ecrire(fds, "\nwith good robots.txt : ");
  ecrireInti(fds, robotsOK, "%10d");

  ecrire(fds, "\n\nsites ready      : ");
  ecrireInti(fds,
             global::okSites->getLength()
             + global::nb_conn
             - global::freeConns->getLength(), "%5d");
  ecrire(fds, "\nsites without ip : ");
  ecrireInti(fds, global::dnsSites->getLength(), "%5d");
}
#endif // NOSTATS

/** draw graphs
 */
#if defined (NOSTATS) || !defined (GRAPH)
#define writeGraph(fds) ((void) 0)
#else
static void writeGraph (int fds) {
  ecrire(fds, "\n\n<h2>Histograms :</h2>");
  histoWrite(fds);
}
#endif

/* write debug information (if any)
 */
#ifdef NDEBUG
#define writeDebug(fds) ((void) 0)
#else
static void writeDebug (int fds) {
  ecrire(fds, "\n\n<h2>Ressources Sharing :</h2>\n\nused connexions : ");
  ecrireInti(fds, global::nb_conn - global::freeConns->getLength(), "%8d");
#ifdef THREAD_OUTPUT
  ecrire(fds, "\nuser connexions : ");
  ecrireInti(fds, global::userConns->getLength(), "%8d");
#endif
  ecrire(fds, "\nfree connexions : ");
  ecrireInti(fds, global::freeConns->getLength(), "%8d");
  ecrire(fds, "\nparsers         : ");
  ecrireInti(fds, debPars, "%8d");
  ecrire(fds, "\n\nsites in ram    : ");
  ecrireInti(fds, sites, "%8d");
  ecrire(fds, "\nipsites in ram  : ");
  ecrireInti(fds, ipsites, "%8d");
  ecrire(fds, "\nurls in ram     : ");
  ecrireInti(fds, debUrl, "%8d");
  ecrire(fds, "   (");
  ecrireInt(fds, global::inter->getPos() - space);
  ecrire(fds, " = ");
  ecrireInt(fds, namedUrl);
  ecrire(fds, " + ");
  ecrireInt(fds, global::IPUrl);
  ecrire(fds, ")\nurls on disk    : ");
  int ui = global::URLsDisk->getLength();
  int uiw = global::URLsDiskWait->getLength();
  ecrireInti(fds, ui+uiw, "%8d");
  ecrire(fds, "   (");
  ecrireInt(fds, ui);
  ecrire(fds, " + ");
  ecrireInt(fds, uiw);
  ecrire(fds, ")\npriority urls   : ");
  int up = global::URLsPriority->getLength();
  int upw = global::URLsPriorityWait->getLength();
  ecrireInti(fds, up+upw, "%8d");
  ecrire(fds, "   (");
  ecrireInt(fds, up);
  ecrire(fds, " + ");

  ecrireInt(fds, upw);
  ecrire(fds, ")\nmiss urls       : ");
  ecrireInti(fds, missUrl, "%8d");

  ecrire(fds, "\n\nreceive : ");
  ecrireIntl(fds, byte_read, "%12lu");
  ecrire(fds, "   (current rate : ");
  ecrireIntl(fds, readRate, "%7lu");
  ecrire(fds, ", overall rate : ");
  ecrireIntl(fds, byte_read / totalduree, "%7lu");
  ecrire(fds, ")\nemit    : ");
  ecrireIntl(fds, byte_write, "%12lu");
  ecrire(fds, "   (current rate : ");
  ecrireIntl(fds, writeRate, "%7lu");
  ecrire(fds, ", overall rate : ");
  ecrireIntl(fds, byte_write / totalduree, "%7lu");

  ecrire(fds, ")\n\nstateMain : ");
  ecrireInt(fds, stateMain);
  ecrire(fds, "\ndebug     : ");
  ecrireInt(fds, debug);

#ifdef HAS_PROC_SELF_STATUS
  ecrire(fds, "\n\n<h2>/proc/self/status :</h2>\n");
  int status = open("/proc/self/status", O_RDONLY);
  char *file = readfile(status);
  ecrire(fds, file);
  delete [] file;
  close(status);
#endif // HAS_PROC_SELF_STATUS
}
#endif // NDEBUG

/* write urls of the first dnsSites
 */
static void writeUrls (int fds) {
  ecrire(fds,"<h2>Urls in next NamedSite</h2>\n");
  NamedSite *ds = global::dnsSites->tryRead();
  if (ds != NULL) {
    ecrireInt(fds, ds->fifoLength());
    ecrire(fds, " waiting url(s)\n\n");
    for (uint i=ds->outFifo; i!=ds->inFifo; i=(i+1)%maxUrlsBySite) {
      int n = ds->fifo[i]->writeUrl(buf);
      buf[n++] = '\n';
      ecrireBuff(fds, buf, n);
    }
  } else {
    ecrire(fds, "no site available\n");
  }
}

/* write urls of the first site in okSites
 */
static void writeIpUrls (int fds) {
  ecrire(fds,"<h2>Urls in next IPSite</h2>\n");
  IPSite *is = global::okSites->tryRead();
  if (is != NULL) {
    Fifo<url> *f = &is->tab;
    ecrireInt(fds, f->getLength());
    ecrire(fds, " waiting url(s)\n\n");
    for (uint i=f->out; i!=f->in; i=(i+1)%f->size) {
      int n = f->tab[i]->writeUrl(buf);
      buf[n++] = '\n';
      ecrireBuff(fds, buf, n);
    }
  } else {
    ecrire(fds, "no site available\n");
  }
}

/* main function, manages the dispatch
 */
static void manageAns (int fds, char *req) {
  if (req != NULL) {
    writeHeader(fds);
    if (!strncmp(req, "/output.html", 12)) {
      outputStats(fds);
    } else if (!strcmp(req, "/dns.html")) {
      writeUrls(fds);
    } else if (!strcmp(req, "/ip.html")) {
      writeIpUrls(fds);
    } else if (!strcmp(req, "/all.html")) {
      writeTime(fds);
      writeStats(fds);
      writeGraph(fds);
      writeDebug(fds);
    } else if (!strcmp(req, "/debug.html")) {
      writeTime(fds);
      writeDebug(fds);
    } else if (!strcmp(req, "/graph.html")) {
      writeTime(fds);
      writeGraph(fds);
    } else if (!strcmp(req, "/smallstats.html")) {
      writeTime(fds);
      writeStats(fds);
    } else { // stat
      writeTime(fds);
      writeStats(fds);
      writeGraph(fds);
    }
    writeFooter(fds);
  }
}

/******************************************************************/
// read request
static int pos;
static bool endFile;

/** parse and answer the current state
 * 1 : continue
 * 0 : end
 * -1 : error
 */
static int parseRequest (int size) {
  while (size > 0) {
    if (pos < 4) {
      if (buf[pos] != "GET "[pos]) {
        return -1;
      } else {
        pos++; size--;
      }
    } else {
      if (endFile) {
        // see if the request if finished
        pos += size; size = 0;
        if (buf[pos-1] == '\n' && buf[pos-3] == '\n') {
          return 0;
        }
      } else {
        // find end of file
        char *tmp = strchr(buf+pos, ' ');
        if (tmp == NULL) {
          pos += size;
          return 1;
        } else {
          *tmp = 0;
          int i = pos;
          pos = tmp + 1 - buf;
          size = size + i - pos;
          endFile = true;
        }
      }
    }
  }
  return 1;
}

static char *readRequest (int fds) {
  pos = 0;
  endFile = false;
  while (true) {
    int size = read(fds, buf+pos, BUF_SIZE-pos);
    switch (size) {
    case -1:
    case 0:
      return NULL;
    default:
      buf[pos+size] = 0;
      int cont = parseRequest(size);
      if (cont == -1) {
        return NULL;
      } else if (cont == 0) {
        return buf+4;
      }
    }
  }
  // never reached, but avoid gcc warning
  return NULL;
}

#endif // NOWEBSERVER
