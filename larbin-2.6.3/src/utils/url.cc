// Larbin
// Sebastien Ailleret
// 15-11-99 -> 16-03-02

/* This class describes an URL */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "options.h"

#include "types.h"
#include "global.h"
#include "utils/url.h"
#include "utils/text.h"
#include "utils/connexion.h"
#include "utils/debug.h"

#ifdef COOKIES
#define initCookie() cookie=NULL
#else // COOKIES
#define initCookie() ((void) 0)
#endif // COOKIES

/* small functions used later */
static uint siteHashCode (char *host) {
  uint h=0;
  uint i=0;
  while (host[i] != 0) {
	h = 37*h + host[i];
    i++;
  }
  return h % namedSiteListSize;
}

/* return the int with correspond to a char
 * -1 if not an hexa char */
static int int_of_hexa (char c) {
  if (c >= '0' && c <= '9')
    return (c - '0');
  else if (c >= 'a' && c <= 'f')
    return (c - 'a' + 10);
  else if (c >= 'A' && c <= 'F')
    return (c - 'A' + 10);
  else
    return -1;
}

/* normalize a file name : also called by robots.txt parser
 * return true if it is ok, false otherwise (cgi-bin)
 */
bool fileNormalize (char *file) {
  int i=0;
  while (file[i] != 0 && file[i] != '#') {
	if (file[i] == '/') {
	  if (file[i+1] == '.' && file[i+2] == '/') {
		// suppress /./
		int j=i+3;
		while (file[j] != 0) {
		  file[j-2] = file[j];
		  j++;
		}
		file[j-2] = 0;
      } else if (file[i+1] == '/') {
        // replace // by /
        int j=i+2;
        while (file[j] != 0) {
          file[j-1] = file[j];
		  j++;
        }
        file[j-1] = 0;
      } else if (file[i+1] == '.' && file[i+2] == '.' && file[i+3] == '/') {
		// suppress /../
		if (i == 0) {
		  // the file name starts with /../ : error
		  return false;
		} else {
		  int j = i+4, dec;
		  i--;
		  while (file[i] != '/') { i--; }
		  dec = i+1-j; // dec < 0
		  while (file[j] != 0) {
			file[j+dec] = file[j];
			j++;
		  }
		  file[j+dec] = 0;
		}
	  } else if (file[i+1] == '.' && file[i+2] == 0) {
		// suppress /.
        file[i+1] = 0;
        return true;
	  } else if (file[i+1] == '.' && file[i+2] == '.' && file[i+3] == 0) {
		// suppress /..
		if (i == 0) {
          // the file name starts with /.. : error
		  return false;
		} else {
		  i--;
		  while (file[i] != '/') {
			i--;
		  }
          file[i+1] = 0;
          return true;
		}
	  } else { // nothing special, go forward
		i++;
	  }
	} else if (file[i] == '%') {
      int v1 = int_of_hexa(file[i+1]);
      int v2 = int_of_hexa(file[i+2]);
      if (v1 < 0 || v2 < 0) return false;
      char c = 16 * v1 + v2;
      if (isgraph(c)) {
        file[i] = c;
        int j = i+3;
        while (file[j] != 0) {
          file[j-2] = file[j];
          j++;
        }
        file[j-2] = 0;
        i++;
      } else if (c == ' ' || c == '/') { // keep it with the % notation
        i += 3;
      } else { // bad url
        return false;
      }
    } else { // nothing special, go forward
	  i++;
	}
  }
  file[i] = 0;
  return true;
}

/**************************************/
/* definition of methods of class url */
/**************************************/

/* Constructor : Parses an url */
url::url (char *u, int8_t depth, url *base) {
  newUrl();
  this->depth = depth;
  host = NULL;
  port = 80;
  file = NULL;
  initCookie();
#ifdef URL_TAGS
  tag = 0;
#endif // URL_TAGS
  if (startWith("http://", u)) {
	// absolute url
	parse (u + 7);
    // normalize file name
    if (file != NULL && !normalize(file)) {
      delete [] file;
      file = NULL;
      delete [] host;
      host = NULL;
    }
  } else if (base != NULL) {
	if (startWith("http:", u)) {
	  parseWithBase(u+5, base);
	} else if (isProtocol(u)) {
	  // Unknown protocol (mailto, ftp, news, file, gopher...)
	} else {
	  parseWithBase(u, base);
	}
  }
}

/* constructor used by input */
url::url (char *line,  int8_t depth) {
  newUrl();
  this->depth = depth;
  host = NULL;
  port = 80;
  file = NULL;
  initCookie();
  int i=0;
#ifdef URL_TAGS
  tag = 0;
  while (line[i] >= '0' && line[i] <= '9') {
    tag = 10*tag + line[i] - '0';
    i++;
  }
  i++;
#endif // URL_TAGS
  if (startWith("http://", line+i)) {
    parse(line+i+7);
    // normalize file name
    if (file != NULL && !normalize(file)) {
      delete [] file;
      file = NULL;
      delete [] host;
      host = NULL;
    }
  }
}

/* Constructor : read the url from a file (cf serialize)
 */
url::url (char *line) {
  newUrl();
  int i=0;
  // Read depth
  depth = 0;
  while (line[i] >= '0' && line[i] <= '9') {
    depth = 10*depth + line[i] - '0';
    i++;
  }
#ifdef URL_TAGS
  // read tag
  tag = 0; i++;
  while (line[i] >= '0' && line[i] <= '9') {
    tag = 10*tag + line[i] - '0';
    i++;
  }
#endif // URL_TAGS
  int deb = ++i;
  // Read host
  while (line[i] != ':') {
    i++;
  }
  line[i] = 0;
  host = newString(line+deb);
  i++;
  // Read port
  port = 0;
  while (line[i] >= '0' && line[i] <= '9') {
    port = 10*port + line[i] - '0';
    i++;
  }
#ifndef COOKIES
  // Read file name
  file = newString(line+i);
#else // COOKIES
  char *cpos = strchr(line+i, ' ');
  if (cpos == NULL) {
    cookie = NULL;
  } else {
    *cpos = 0;
    // read cookies
    cookie = new char[maxCookieSize];
    strcpy(cookie, cpos+1);
  }
  // Read file name
  file = newString(line+i);
#endif // COOKIES
}

/* constructor used by giveBase */
url::url (char *host, uint port, char *file) {
  newUrl();
  initCookie();
  this->host = host;
  this->port = port;
  this->file = file;
}

/* Destructor */
url::~url () {
  delUrl();
  delete [] host;
  delete [] file;
#ifdef COOKIES
  delete [] cookie;
#endif // COOKIES
}

/* Is it a valid url ? */
bool url::isValid () {
  if (host == NULL) return false;
  int lh = strlen(host);
  return file!=NULL && lh < maxSiteSize
    && lh + strlen(file) + 18 < maxUrlSize;
}

/* print an URL */
void url::print () {
  printf("http://%s:%u%s\n", host, port, file);
}

/* Set depth to max if necessary
 * try to find the ip addr
 * answer false if forbidden by robots.txt, true otherwise */
bool url::initOK (url *from) {
#if defined(DEPTHBYSITE) || defined(COOKIES)
  if (strcmp(from->getHost(), host)) { // different site
#ifdef DEPTHBYSITE
	depth = global::depthInSite;
#endif // DEPTHBYSITE
  } else { // same site
#ifdef COOKIES
    if (from->cookie != NULL) {
      cookie = new char[maxCookieSize];
      strcpy(cookie, from->cookie);
    }
#endif // COOKIES
  }
#endif // defined(DEPTHBYSITE) || defined(COOKIES)
  if (depth < 0) {
    errno = tooDeep;
    return false;
  }
  NamedSite *ns = global::namedSiteList + (hostHashCode());
  if (!strcmp(ns->name, host) && ns->port == port) {
    switch (ns->dnsState) {
    case errorDns:
      errno = fastNoDns;
      return false;
    case noConnDns:
      errno = fastNoConn;
      return false;
    case doneDns:
      if (!ns->testRobots(file)) {
        errno = fastRobots;
        return false;
      }
    }
  }
  return true;
}

/* return the base of the url */
url *url::giveBase () {
  int i = strlen(file);
  assert (file[0] == '/');
  while (file[i] != '/') {
	i--;
  }
  char *newFile = new char[i+2];
  memcpy(newFile, file, i+1);
  newFile[i+1] = 0;
  return new url(newString(host), port, newFile);
}

/** return a char * representation of the url
 * give means that you have to delete the string yourself
 */
char *url::giveUrl () {
  char *tmp;
  int i = strlen(file);
  int j = strlen(host);

  tmp = new char[18+i+j];  // 7 + j + 1 + 9 + i + 1
                           // http://(host):(port)(file)\0
  strcpy(tmp, "http://");
  strcpy (tmp+7, host);
  j += 7;
  if (port != 80) {
    j += sprintf(tmp + j, ":%u", port);
  }
  // Copy file name
  while (i >= 0) {
	tmp [j+i] = file[i];
	i--;
  }
  return tmp;
}

/** write the url in a buffer
 * buf must be at least of size maxUrlSize
 * returns the size of what has been written (not including '\0')
 */
int url::writeUrl (char *buf) {
  if (port == 80)
    return sprintf(buf, "http://%s%s", host, file);
  else
    return sprintf(buf, "http://%s:%u%s", host, port, file);
}

/* serialize the url for the Persistent Fifo */
char *url::serialize () {
  // this buffer is protected by the lock of PersFifo
  static char statstr[maxUrlSize+40+maxCookieSize];
  int pos = sprintf(statstr, "%u ", depth);
#ifdef URL_TAGS
  pos += sprintf(statstr+pos, "%u ", tag);
#endif // URL_TAGS
  pos += sprintf(statstr+pos, "%s:%u%s", host, port, file);
#ifdef COOKIES
  if (cookie != NULL) {
    pos += sprintf(statstr+pos, " %s", cookie);
  }
#endif // COOKIES
  statstr[pos] = '\n';
  statstr[pos+1] = 0;
  return statstr;
}

/* very thread unsafe serialisation in a static buffer */
char *url::getUrl() {
  static char statstr[maxUrlSize+40];
  sprintf(statstr, "http://%s:%u%s", host, port, file);
  return statstr;
}

/* return a hashcode for the host of this url */
uint url::hostHashCode () {
  return siteHashCode (host);
}

/* return a hashcode for this url */
uint url::hashCode () {
  unsigned int h=port;
  unsigned int i=0;
  while (host[i] != 0) {
	h = 31*h + host[i];
    i++;
  }
  i=0;
  while (file[i] != 0) {
	h = 31*h + file[i];
    i++;
  }
  return h % hashSize;
}

/* parses a url : 
 * at the end, arg must have its initial state, 
 * http:// has allready been suppressed
 */
void url::parse (char *arg) {
  int deb = 0, fin = deb;
  // Find the end of host name (put it into lowerCase)
  while (arg[fin] != '/' && arg[fin] != ':' && arg[fin] != 0) {
	fin++;
  }
  if (fin == 0) return;

  // get host name
  host = new char[fin+1];
  for (int  i=0; i<fin; i++) {
    host[i] = lowerCase(arg[i]);
  }
  host[fin] = 0;

  // get port number
  if (arg[fin] == ':') {
	port = 0;
    fin++;
	while (arg[fin] >= '0' && arg[fin] <= '9') {
	  port = port*10 + arg[fin]-'0';
	  fin++;
	}
  }

  // get file name
  if (arg[fin] != '/') {
	// www.inria.fr => add the final /
    file = newString("/");
  } else {
    file = newString(arg + fin);
  }
}

/** parse a file with base
 */
void url::parseWithBase (char *u, url *base) {
  // cat filebase and file
  if (u[0] == '/') {
    file = newString(u);
  } else {
    uint lenb = strlen(base->file);
    char *tmp = new char[lenb + strlen(u) + 1];
    memcpy(tmp, base->file, lenb);
    strcpy(tmp + lenb, u);
    file = tmp;
  }
  if (!normalize(file)) {
    delete [] file;
    file = NULL;
    return;
  }
  host = newString(base->host);
  port = base->port;
}

/** normalize file name
 * return true if it is ok, false otherwise (cgi-bin)
 */
bool url::normalize (char *file) {
  return fileNormalize(file);
}

/* Does this url starts with a protocol name */
bool url::isProtocol (char *s) {
  uint i = 0;
  while (isalnum(s[i])) {
	i++;
  }
  return s[i] == ':';
}

#ifdef COOKIES
#define addToCookie(s) len = strlen(cookie); \
    strncpy(cookie+len, s, maxCookieSize-len); \
    cookie[maxCookieSize-1] = 0;

/* see if a header contain a new cookie */
void url::addCookie(char *header) {
  if (startWithIgnoreCase("set-cookie: ", header)) {
    char *pos = strchr(header+12, ';');
    if (pos != NULL) {
      int len;
      if (cookie == NULL) {
        cookie = new char[maxCookieSize];
        cookie[0] = 0;
      } else {
        addToCookie("; ");
      }
      *pos = 0;
      addToCookie(header+12);
      *pos = ';';
    }
  }
}
#endif // COOKIES
