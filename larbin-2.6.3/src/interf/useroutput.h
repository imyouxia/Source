// Larbin
// Sebastien Ailleret
// 07-12-01 -> 03-01-02

#ifndef USEROUTPUT_H
#define USEROUTPUT_H

#include "utils/url.h"
#include "fetch/file.h"

/** A page has been loaded successfully
 * @param page the page that has been fetched
 */
void loaded (html *page);
  // This function should manage anything
  // page->getHeaders() gives a char* containing the http headers
  // page->getPage() gives a char* containing the page itself
  // those char* are statically allocated, so you should copy
  // them if you want to keep them
  // in order to accept \000 in the page, you can use page->getLength()

/** The fetch failed
 * @param u the URL of the doc
 * @param reason reason of the fail
 */
void failure (url *u, FetchError reason);

/** initialisation function
 * This function is called at the end of global initialisation
 */
void initUserOutput ();

/** stats, called in particular by the webserver
 * the webserver is in another thread, so be careful
 * However, if it only reads things, it is probably not useful
 * to use mutex, because incoherence in the webserver is not as critical
 * as efficiency
 */
void outputStats(int fds);

#endif // USEROUTPUT_H
