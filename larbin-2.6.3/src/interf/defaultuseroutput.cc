// Larbin
// Sebastien Ailleret
// 07-12-01 -> 07-12-01

#include <iostream.h>
#include <string.h>
#include <unistd.h>

#include "options.h"

#include "types.h"
#include "global.h"
#include "fetch/file.h"
#include "utils/text.h"
#include "utils/debug.h"
#include "interf/output.h"

/** A page has been loaded successfully
 * @param page the page that has been fetched
 */
void loaded (html *page) {
  // Here should be the code for managing everything
  // page->getHeaders() gives a char* containing the http headers
  // page->getPage() gives a char* containing the page itself
  // those char* are statically allocated, so you should copy
  // them if you want to keep them
  // in order to accept \000 in the page, you can use page->getLength()
#ifdef BIGSTATS
  cout << "fetched : ";
  page->getUrl()->print();
  // cout << page->getHeaders() << "\n" << page->getPage() << "\n";
#endif // BIGSTATS
}

/** The fetch failed
 * @param u the URL of the doc
 * @param reason reason of the fail
 */
void failure (url *u, FetchError reason) {
  // Here should be the code for managing everything
#ifdef BIGSTATS
  cout << "fetched failed (" << (int) reason << ") : ";
  u->print();
#endif // BIGSTATS
}

/** initialisation function
 */
void initUserOutput () {

}

/** stats, called in particular by the webserver
 * the webserver is in another thread, so be careful
 * However, if it only reads things, it is probably not useful
 * to use mutex, because incoherence in the webserver is not as critical
 * as efficiency
 */
void outputStats(int fds) {
  ecrire(fds, "Nothing to declare");
}
