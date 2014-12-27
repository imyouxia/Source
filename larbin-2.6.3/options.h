// Larbin
// Sebastien Ailleret
// 27-05-01 -> 09-03-02

#ifndef LARBIN_CONFIG
#define LARBIN_CONFIG

#include "config.h"

/* This files allows a lot of customizations of larbin
 * see doc/custom-eng.html for more details
 */

/////////////////////////////////////////////////////////////
// Select the output module you want to use

#define DEFAULT_OUTPUT   // do nothing...
//#define SIMPLE_SAVE      // save in files named save/dxxxxxx/fyyyyyy
//#define MIRROR_SAVE      // save in files (respect sites hierarchy)
//#define STATS_OUTPUT     // do some stats on pages

////////////////////////////////////////////////////////////
// Set up a specific search

//#define SPECIFICSEARCH
//#define contentTypes ((char *[]) { "audio/mpeg", NULL })
//#define privilegedExts ((char *[]) { ".mp3", NULL })

// how do you want to manage specific pages (select one of the followings)
//#define DEFAULT_SPECIFIC
//#define SAVE_SPECIFIC
//#define DYNAMIC_SPECIFIC


//////////////////////////////////////////////////////////
// What do you want the crawler to do

// do you want to follow links in pages
#define FOLLOW_LINKS

// do you want the crawler to associate to each page the list of its sons
//#define LINKS_INFO

// do you want to associate a tag to pages (given in input)
// this allows to follow a page from input to output (and follow redirection)
//#define URL_TAGS

// do you want to suppress duplicate pages
//#define NO_DUP

// do you want larbin to stop when everything has been fetched
//#define EXIT_AT_END

// do you want to fetch images
// if you enable this option, update forbiddenExtensions in larbin.conf
//#define IMAGES

// downlaod everything (ie no check of content type in http headers)
//#define ANYTYPE

// do you want to manage cookies
//#define COOKIES


//////////////////////////////////////////////////////////
// Various options

// do you want to get cgi
// 0 : yes ; 1 : no ; 2 : NO !
#define CGILEVEL 1

// limit bandwith usage (in octets/sec)
// be carefull, larbin might use 10 to 20% more
//#define MAXBANDWIDTH 200000

// the depth is initialized each time a link goes to another site
#define DEPTHBYSITE


//////////////////////////////////////////////////////////
// Efficiency vs feature

// do we need a special thread for output
// This is compulsory if it can block
// (not needed if you did not add code yourself)
//#define THREAD_OUTPUT

// if this option is set, larbin saves the hashtable from time to time
// this way it can restart from where it last stopped
// by reloading the table
//#define RELOAD


//////////////////////////////////////////////////////////
// now it's just if you need to know how it works

// do not launch the webserver
// this can be usefull in order to launch no thread at all
//#define NOWEBSERVER

// do you want nice graphs for in the stats page
#define GRAPH

// uncomment if you are not interested in debugging information
//#define NDEBUG

// enable this if you really dislike stats (in the webserver)
//#define NOSTATS

// enable this if you really like stats (on stdout)
//#define STATS
//#define BIGSTATS

// Please enable this option if you want to report a crash
// then compile with "make debug"
//#define CRASH

#endif // LARBIN_CONFIG
