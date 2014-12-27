// Larbin
// Sebastien Ailleret
// 03-01-02 -> 04-01-02

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

#define nb 25
#define taille 4096
#define larg 40

static double maxs = 0;
static double tabs[nb];

static double maxb = 0;
static double tabb[nb];

static uint64_t totalpages = 1;
static double totalbytes = 0;

/** A page has been loaded successfully
 * @param page the page that has been fetched
 */
void loaded (html *page) {
  uint32_t l = page->getLength();
  int t = l / taille;
  if (t >= nb) {
    t = nb-1;
  }
  tabs[t]++;
  if (tabs[t] > maxs) maxs = tabs[t];
  tabb[t] += (double) l;
  if (tabb[t] > maxb) maxb = tabb[t];
  totalpages++;
  totalbytes += (double) l;
}

/** The fetch failed
 * @param u the URL of the doc
 * @param reason reason of the fail
 */
void failure (url *u, FetchError reason) {
}

/** initialisation function
 */
void initUserOutput () {
  for (int i=0; i<nb; i++) {
    tabs[i] = 0;
    tabb[i] = 0;
  }
}

/** stats, called in particular by the webserver
 * the webserver is in another thread, so be careful
 * However, if it only reads things, it is probably not useful
 * to use mutex, because incoherence in the webserver is not as critical
 * as efficiency
 */
static void dessine(int fds, double *tab, double *maxi) {
  for (int i=0; i<nb; i++) {
    ecrire(fds, "|");
    int n = (int) ((tab[i] * larg) / (*maxi+1));
    for (int j=0; j<n; j++) ecrire(fds, "*");
    ecrire(fds, "\n");
  }
}

void outputStats(int fds) {
  ecrire(fds, "Stats for ");
  ecrireInt(fds, totalpages);
  ecrire(fds, " pages.\nMean size of a page : ");
  ecrireInt(fds, ((int) totalbytes) / totalpages);
  ecrire(fds, "\n\nProportion of pages per size (one row is ");
  ecrireInt(fds, taille);
  ecrire(fds, " bytes, max size is ");
  ecrireInt(fds, taille*nb);
  ecrire(fds, " bytes) :\n\n");
  dessine(fds, tabs, &maxs);
  ecrire(fds, "\n\nbytes transfered by size :\n\n");
  dessine(fds, tabb, &maxb);
}
