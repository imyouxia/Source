// Larbin
// Sebastien Ailleret
// 13-12-99 -> 19-03-02

#ifndef FILE_H
#define FILE_H

#include "options.h"

#include "types.h"
#include "utils/url.h"
#include "utils/string.h"
#include "utils/Vector.h"
#include "fetch/site.h"

struct Connexion;

class file {
 protected:
  // link to the buffer of our connexion
  char *buffer;
  // parsing position
  char *posParse;
 public:
  // Constructor
  file (Connexion *conn);
  // Destructor
  virtual ~file ();
  // Is it a robots.txt
  bool isRobots;
  // current position in the buffer
  uint pos;
  // a string arrives from the server
  virtual int inputHeaders (int size) = 0; // just parse headers
  virtual int endInput () = 0;
};

class html : public file {
 private:
  // Where are we
  url *here;
  // beginning of the current interesting area
  char *area;
  // begining of the real content (end of the headers + 1)
  char *contentStart;
  // base de l'URL
  url *base;
  /* manage a new url : verify and send it */
  void manageUrl (url *nouv, bool isRedir);

  /* All the following functions are used for parsing
   * they return 0 if OK, 1 if problem occurs (errno is set) */
  // parse the answer code line
  int parseCmdline ();
  // parse a line of header (ans 30X) => just look for location
  int parseHeader30X ();
  // parse a line of header
  int parseHeader ();
  // functions for parsing headers called by parseHeader
  int verifType ();
  int verifLength ();
  /* The following functions are called by endInput
   * for parsing the content of the file */
  // enter a html section
  void parseHtml ();
  // enter a comment
  void parseComment ();
  // enter a tag
  void parseTag ();
  // enter a tag content
  void parseContent (int action);

#ifdef LINKS_INFO
  /* links extracted from this page */
  Vector<char> links;
#endif // LINKS_INFO

#include "fetch/specbuf.h"

 public:
  // Constructor
  html (url *here, Connexion *conn);
  // Destructor
  ~html ();
  /** a string is arriving
   * return 0 usually, 1 if don't want any more input
   * in the latter case, errno is set to FetchError reason
   */
  int inputHeaders (int size); // just parse headers
  int endInput ();
  // State of our read : answer, headers, tag, html...
  int state;
  /** return the url of this file */
  inline url *getUrl () { return here; }
  /** Is this page interesting ? */
  bool isInteresting;
  /** return the content of the page */
  char *getPage ();
  int getLength ();
  /** return the http headers */
  inline char *getHeaders () { return buffer; }

#ifdef LINKS_INFO
  /** return the links */
  inline Vector<char> *html::getLinks () { return &links; }
#endif // LINKS_INFO
};

class robots : public file {
 private:
  // did we parse the anser code
  bool answerCode;
  // test http headers
  bool parseHeaders ();
  // read the file
  void parseRobots ();
 public:
  // Constructor
  robots (NamedSite *server, Connexion *conn);
  // Destructor
  ~robots ();
  // for which site is this robots
  NamedSite *server;
  // a string is arriving from the server
  int inputHeaders (int size); // just parse headers
  int endInput ();
  // parse the file (once everything has been read)
  void parse (bool isError);
};

// initialisation in case of specific
void initSpecific ();

#endif // FILE_H
