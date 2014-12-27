// Larbin
// Sebastien Ailleret
// 14-12-99 -> 19-03-02

#include <unistd.h>
#include <iostream.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "options.h"

#include "types.h"
#include "global.h"
#include "utils/text.h"
#include "utils/url.h"
#include "utils/string.h"
#include "utils/Vector.h"
#include "fetch/site.h"
#include "fetch/file.h"
#include "fetch/fetchOpen.h"
#include "fetch/checker.h"

#include "utils/debug.h"

#define ANSWER 0
#define HEADERS 1
#define HEADERS30X 2
#define HTML 3
#define SPECIFIC 4

#define LINK 0
#define BASE 1


/***********************************
 * implementation of file
 ***********************************/

file::file (Connexion *conn) {
  buffer = conn->buffer;
  pos = 0;
  posParse = buffer;
}

file::~file () {
}

/***********************************
 * implementation of robots
 ***********************************/

/** Constructor
 */
robots::robots (NamedSite *server, Connexion *conn) : file(conn) {
  newPars();
  this->server = server;
  answerCode = false;
  isRobots = true;
}

/** Destructor
 */
robots::~robots () {
  delPars();
  // server is not deleted on purpose
  // it belongs to someone else
}

/** we get some more chars of this file
 */
int robots::endInput () {
  return 0;
}

/** input and parse headers
 */
int robots::inputHeaders (int size) {
  pos += size;
  if (!answerCode && pos > 12) {
    if (buffer[9] == '2') {
      answerCode = true;
    } else {
      errno = err40X;
      return 1;
    }
  }
  if (pos > maxRobotsSize) {
	// no more input, forget the end of this file
	errno = tooBig;
	return 1;
  } else {
	return 0;
  }
}

/** parse the robots.txt
 */
void robots::parse (bool isError) {
  if (answerCode && parseHeaders()) {
    siteRobots();
    buffer[pos] = 0;
	if (isError) {
	  // The file could be incomplete, delete last token
	  // We could have Disallow / instead of Disallow /blabla
	  for (uint i=pos-1; i>0 && !isspace(buffer[i]); i--) {
		buffer[i] = ' ';
	  }
	}
    parseRobots();
  }
}

/** test http headers
 * return true if OK, false otherwise
 */
bool robots::parseHeaders () {
  for(posParse = buffer+9; posParse[3] != 0; posParse++) {
    if ((posParse[0] == '\n' && 
         (posParse[1] == '\n'
          || posParse[2] == '\n'))
        || (posParse[0] == '\r' &&
            (posParse[1] == '\r'
             || posParse[2] == '\r'))) {
      return true;
    }
  }
  return false;
}

/** try to understand the file
 */
void robots::parseRobots () {
  robotsOK();
#ifndef NOSTATS
  bool goodfile = true;
#endif // NOSTATS
  server->forbidden.recycle();
  uint items = 0; // size of server->forbidden
  // state
  // 0 : not concerned
  // 1 : weakly concerned
  // 2 : strongly concerned
  int state = 1;
  char *tok = nextToken(&posParse, ':');
  while (tok != NULL) {
	if (!strcasecmp(tok, "useragent") || !strcasecmp(tok, "user-agent")) {
	  if (state == 2) {
		// end of strong concern record => the end for us
		return;
	  } else {
		state = 0;
		// what is the new state ?
		tok = nextToken(&posParse, ':');
		while (tok != NULL
			   && strcasecmp(tok, "useragent")
			   && strcasecmp(tok, "user-agent")
			   && strcasecmp(tok, "disallow")) {
          if (caseContain(tok, global::userAgent)) {
            state = 2;
          } else if (state == 0 && !strcmp(tok, "*")) {
            state = 1;
          }
		  tok = nextToken(&posParse, ':');
		}
	  }
	  if (state) {
		// delete old forbidden : we've got a better record than older ones
		server->forbidden.recycle();
		items = 0;
	  } else {
        // forget this record
        while (tok != NULL
			   && strcasecmp(tok, "useragent")
			   && strcasecmp(tok, "user-agent")) {
          tok = nextToken(&posParse, ':');
        }
      }
	} else if (!strcasecmp(tok, "disallow")) {
      tok = nextToken(&posParse, ':');
      while (tok != NULL
             && strcasecmp(tok, "useragent")
             && strcasecmp(tok, "user-agent")
             && strcasecmp(tok, "disallow")) {
        // add nextToken to forbidden
        if (items++ < maxRobotsItem) {
          // make this token a good token
          if (tok[0] == '*') { // * is not correct, / disallows everything
            tok[0] = '/';
          } else if (tok[0] != '/') {
            tok--;
            tok[0] = '/';
          }
          if (fileNormalize(tok)) {
            server->forbidden.addElement(newString(tok));
          }
        }
        tok = nextToken(&posParse, ':');
      }
	} else {
#ifndef NOSTATS
	  if (goodfile) {
        robotsOKdec();
		goodfile = false;
	  }
#endif // NOSTATS
	  tok = nextToken(&posParse, ':');
	}
  }
}


/*************************************
 * implementation of html
 *************************************/


/////////////////////////////////////////
#ifdef SPECIFICSEARCH

#include "fetch/specbuf.cc"

#define _newSpec() if (state==SPECIFIC) newSpec()
#define _destructSpec() if (state==SPECIFIC) destructSpec()
#define _endOfInput() if (state==SPECIFIC) return endOfInput()
#define _getContent() \
  if (state==SPECIFIC) return getContent(); \
  else return contentStart
#define _getSize() \
  if (state==SPECIFIC) return getSize(); \
  else return (buffer + pos - contentStart)

///////////////////////////////////////
#else // not a SPECIFICSEARCH

void initSpecific () { }

#define constrSpec() ((void) 0)
#define _newSpec() ((void) 0)
#define pipeSpec() 0
#define _endOfInput() ((void) 0)
#define _destructSpec() ((void) 0)
#define _getContent() return contentStart
#define _getSize() return (buffer + pos - contentStart)

#endif // SPECIFICSEARCH
/////////////////////////////////////////

#if CGILEVEL >= 1
#define notCgiChar(c) (c!='?' && c!='=' && c!='*')
#else
#define notCgiChar(c) true
#endif // CGILEVEL

/** Constructor
 */
html::html (url *here, Connexion *conn) : file(conn) {
  newPars();
  this->here = here;
  base = here->giveBase();
  state = ANSWER;
  isInteresting = false;
  constrSpec();
  pages();
  isRobots = false;
}

/** Destructor
 */
html::~html () {
  _destructSpec();
  delPars();
  delete here;
  delete base;
}

/* get the content of the page */
char *html::getPage () {
  _getContent();
}

int html::getLength () {
  _getSize();
}

/* manage a new url : verify and send it */
void html::manageUrl (url *nouv, bool isRedir) {
  if (nouv->isValid()
      && filter1(nouv->getHost(), nouv->getFile())
      && (global::externalLinks || isRedir
          || !strcmp(nouv->getHost(), this->here->getHost()))) {
    // The extension is not stupid (gz, pdf...)
#ifdef LINKS_INFO
    links.addElement(nouv->giveUrl());
#endif // LINKS_INFO
    if (nouv->initOK(here)) {
      check(nouv);
    } else {
      // this url is forbidden for errno reason (set by initOK)
      answers(errno);
      delete nouv;
    }
  } else {
    // The extension is stupid
    delete nouv;
  }
}

/**********************************************/
/* This part manages command line and headers */
/**********************************************/

/** a string is arriving, treat it only up to the end of headers
 * return 0 usually, 1 if no more input and set errno accordingly
 */
int html::inputHeaders (int size) {
  pos += size;
  buffer[pos] = 0;
  char *posn;
  while (posParse < buffer + pos) {
    switch (state) {
    case ANSWER:
      posn = strchr(posParse, '\n');
      if (posn != NULL) {
        posParse = posn;
        if (parseCmdline ()) {
          return 1;
        }
        area = ++posParse;
      } else {
        return 0;
      }
      break;
    case HEADERS:
    case HEADERS30X:
      posn = strchr(posParse, '\n');
      if (posn != NULL) {
        posParse = posn;
        int tmp;
        if (state == HEADERS)
          tmp = parseHeader();
        else tmp = parseHeader30X();
        if (tmp) {
          return 1;
        }
        area = ++posParse;
      } else {
        return 0;
      }
      break;
    case SPECIFIC:
      return pipeSpec();
    default:
      return 0;
    }
  }
  return 0;
}

/** parse the answer code line */
int html::parseCmdline () {
  if (posParse - buffer >= 12) {
    switch (buffer[9]) {
    case '2':
      state = HEADERS;
      break;
    case '3':
      state = HEADERS30X;
      break;
    default:
      errno = err40X;
      return 1;
    }
  } else {
    errno = earlyStop;
    return 1;
  }
  return 0;
}

/** parse a line of header
 * @return 0 if OK, 1 if we don't want to read the file
 */
int html::parseHeader () {
  if (posParse - area < 2) {
	// end of http headers
#ifndef FOLLOW_LINKS
    state = SPECIFIC;
#elif defined(SPECIFICSEARCH)
    if (isInteresting) {
      state = SPECIFIC;
    } else {
      state = HTML;
    }
#else // not a SPECIFICSEARCH
    state = HTML;
#endif // SPECIFICSEARCH
    contentStart = posParse + 1;
    *(posParse-1) = 0;
    _newSpec();
  } else {
    *posParse = 0;
    here->addCookie(area);
    *posParse = '\n';
    if (verifType ()) return 1;
    if (verifLength()) return 1;
  }
  return 0;
}

/** function called by parseHeader
 * parse content-type
 * return 1 (and set errno) if bad type, 0 otherwise
 * can toggle isInteresting
 */
#define errorType() errno=badType; return 1

#ifdef ANYTYPE
#define checkType() return 0
#elif defined(IMAGES)
#define checkType() if (startWithIgnoreCase("image", area+14)) { \
    return 0; \
  } else { errorType (); }
#else
#define checkType() errorType()
#endif

int html::verifType () {
  if (startWithIgnoreCase("content-type: ", area)) {
    // Let's read the type of this doc
    if (!startWithIgnoreCase("text/html", area+14)) {
#ifdef SPECIFICSEARCH
      if (matchContentType(area+14)) {
        interestingSeen();
        isInteresting = true;
      } else {
        checkType();
      }
#else // SPECIFICSEARCH
      checkType();
#endif // SPECIFICSEARCH
    }
  }
  return 0;
}

/** function called by parseHeader
 * parse content-length
 * return 1 (and set errno) if too long file, 0 otherwise
 */
int html::verifLength () {
#ifndef SPECIFICSEARCH
  if (startWithIgnoreCase("content-length: ", area)) {
    int len = 0;
    char *p = area+16;
    while (*p >= '0' && *p <= '9') {
      len = len*10 + *p -'0';
      p++;
    }
    if (len > maxPageSize) {
      errno = tooBig;
      return 1;
    }
  }
#endif // SPECIFICSEARCH
  return 0;
}

/** parse a line of header (ans 30X) => just look for location
 * @return 0 if OK, 1 if we don't want to read the file
 */
int html::parseHeader30X () {
  if (posParse - area < 2) {
	// end of http headers without location => err40X
    errno = err40X;
    return 1;
  } else {
	if (startWithIgnoreCase("location: ", area)) {
      int i=10;
      while (area[i]!=' ' && area[i]!='\n' && area[i]!='\r'
             && notCgiChar(area[i])) {
        i++;
      }
      if (notCgiChar(area[i])) {
        area[i] = 0; // end of url
        // read the location (do not decrease depth)
        url *nouv = new url(area+10, here->getDepth(), base);
#ifdef URL_TAGS
        nouv->tag = here->tag;
#endif // URL_TAGS
        manageUrl(nouv, true);
        // we do not need more headers
      }
      errno = err30X;
      return 1;
	}
  }
  return 0;
}

/*********************************************/
/* This part manages the content of the file */
/*********************************************/

/** file download is complete, parse the file (headers already done)
 * return 0 usually, 1 if there was an error
 */
int html::endInput () {
  if (state <= HEADERS) {
    errno = earlyStop;
    return 1;
  }
  if (state == HEADERS30X) {
    errno = err40X;
    return 1;
  }
#ifdef NO_DUP
  if (!global::hDuplicate->testSet(posParse)) {
    errno = duplicate;
    return 1;
  }
#endif // NO_DUP
  buffer[pos] = 0;
  _endOfInput();
  // now parse the html
  parseHtml();
  return 0;
}

/* parse an html page */
void html::parseHtml () {
  while ((posParse=strchr(posParse, '<')) != NULL) {
    if (posParse[1] == '!') {
      if (posParse[2] == '-' && posParse[3] == '-') {
        posParse += 4;
        parseComment();
      } else {
        // nothing...
        posParse += 2;
      }
    } else {
      posParse++;
      parseTag();
    }
  }
}

/* skip a comment */
void html::parseComment() {
  while ((posParse=strchr(posParse, '-')) != NULL) {
    if (posParse[1] == '-' && posParse[2] == '>') {
      posParse += 3;
      return;
    } else {
      posParse++;
    }
  }
  posParse = buffer+pos;
}

/* macros used by the following functions */
#define skipSpace() \
  while (*posParse == ' ' || *posParse == '\n' \
         || *posParse == '\r' || *posParse == '\t') { \
    posParse++; \
  }
#define skipText() \
  while (*posParse != ' ' && *posParse != '\n' && *posParse != '>' \
         && *posParse != '\r' && *posParse != '\t' && *posParse != 0) { \
    posParse++; \
  }
#define nextWord() skipText(); skipSpace()
#define thisCharIs(i, c) (c == (posParse[i]|32))
#define isTag(t, p, a, i) if (t) { \
      param = p; \
      action = a; \
      posParse += i; \
    } else { \
      posParse++; \
      return; \
    }

/** Try to understand this tag */
void html::parseTag () {
  skipSpace();
  char *param=NULL; // what parameter are we looking for
  int action=-1;
  // read the name of the tag
  if (thisCharIs(0, 'a')) { // a href
    param = "href";
    action = LINK;
    posParse++;
  } else if (thisCharIs(0, 'l')) {
    isTag(thisCharIs(1, 'i') && thisCharIs(2, 'n') && thisCharIs(3, 'k'),
          "href", LINK, 4);
  } else if (thisCharIs(0, 'b')) { // base href
    isTag(thisCharIs(1, 'a') && thisCharIs(2, 's') && thisCharIs(3, 'e'),
          "href", BASE, 4);
  } else if (thisCharIs(0, 'f')) { // frame src
    isTag(thisCharIs(1, 'r') && thisCharIs(2, 'a')
          && thisCharIs(3, 'm') && thisCharIs(4, 'e'),
          "src", LINK, 5);
#ifdef IMAGES
  } else if (thisCharIs(0, 'i')) { // img src
    isTag(thisCharIs(1, 'm') && thisCharIs(2, 'g'), "src", LINK, 3);
#endif // IMAGES
  } else {
    return;
  }
  // now find the parameter
  assert(param != NULL);
  skipSpace();
  for (;;) {
    int i=0;
    while (param[i]!=0 && thisCharIs(i, param[i])) i++;
    posParse += i;
    if (posParse[i]=='>' || posParse[i]==0) return;
    if (param[i]==0) {
      parseContent(action);
      return;
    } else {
      // not the good parameter
      nextWord();
    }
  }
}

/** read the content of an interesting tag */
void html::parseContent (int action) {
  posParse++;
  while (*posParse==' ' || *posParse=='=') posParse++;
  if (*posParse=='\"' || *posParse=='\'') posParse++;
  area = posParse;
  char *endItem = area + maxUrlSize;
  if (endItem > buffer + pos) endItem = buffer + pos;
  while (posParse < endItem && *posParse!='\"' && *posParse!='\''
         && *posParse!='\n' && *posParse!=' ' && *posParse!='>'
         && *posParse!='\r' && *posParse!='\t' && notCgiChar(*posParse)) {
    if (*posParse == '\\') *posParse = '/';    // Bye Bye DOS !
    posParse++;
  }
  if (posParse == buffer + pos) {
    // end of file => content may be truncated => forget it
    return;
  } else if (posParse < endItem && notCgiChar(*posParse)) {
    // compute this url (not too long and not cgi)
    char oldchar = *posParse;
    *posParse = 0;
    switch (action) {
    case LINK:
      // try to understand this new link
      manageUrl(new url(area, here->getDepth()-1, base), false);
      break;
    case BASE:
      // This page has a BASE HREF tag
      {
        uint end = posParse - area - 1;
        while (end > 7 && area[end] != '/') end--; // 7 because http://
        if (end > 7) { // this base looks good
          end++;
          char tmp = area[end];
          area[end] = 0;
          url *tmpbase = new url(area, 0, (url *) NULL);
          area[end] = tmp;
          delete base;
          if (tmpbase->isValid()) {
            base = tmpbase;
          } else {
            delete tmpbase;
            base = NULL;
          }
        }
      }
      break;
    default: assert(false);
    }
    *posParse = oldchar;
  }
  posParse++;
}
