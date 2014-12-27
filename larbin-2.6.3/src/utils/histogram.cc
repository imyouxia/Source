// Larbin
// Laurent Viennot
// 10-10-01 -> 10-10-01

// modified by Sebastien Ailleret
// 19-10-01 -> 25-10-01

/* histogram of number of pages retrieved for graphical stats */

#include "options.h"

#ifdef GRAPH

#include <string.h>

#include "global.h"
#include "utils/histogram.h"
#include "utils/connexion.h"

#define size 80
#define height 15

static char curve[height+2][size+17];

/* definition of class histogram */
class Histogram {
protected:
  int tab1[size];
  int tab2[size];
  int maxv;
  int beg, end;
  time_t period, count, beg_time;

  /* Insert a new interval : */
  void incrementEnd ();

public:
  /* Specific constructor : the number of pages retrieved during 
   *   an interval is traced
   * @param period : time length of an interval (in seconds)
   * @param size : number of intervals remembered
   */
  Histogram (time_t period);

  /* Destructor */
  ~Histogram ();

  /* A page is retrieved, add to stats */
  void pageHit (int x, int y);

  /* Text output stat for last intervals */
  void write (int fds);
};

/* our histograms */
static Histogram *histoHeure = new Histogram (3600);
static Histogram *histoMinute = new Histogram (60);
static Histogram *histoSeconde = new Histogram (1);

void histoHit (uint x, uint y) {
  static uint lastx = 0, lasty = 0;
  int tmpx = x - lastx;
  int tmpy = y - lasty;
  lastx = x;
  lasty = y;
  histoHeure->pageHit (tmpx, tmpy);
  histoMinute->pageHit (tmpx, tmpy);
  histoSeconde->pageHit (tmpx, tmpx);
}

void histoWrite (int fds) {
  histoHeure->write (fds);
  histoMinute->write (fds);
  histoSeconde->write (fds);
}

/*************************************/
/* Implementation of class histogram */
/*************************************/

/* Specific constructor : the number of pages retrieved during 
 *   an interval is traced
 * @param period : time length of an interval (in seconds)
 * @param size : number of intervals remembered
 */
Histogram::Histogram (time_t period) {
  for (int i=0; i<size; ++i) {
    tab1[i] = 0;
    tab2[i] = 0;
  }
  beg = 0; end = 0;
  maxv = height;
  this->period = period;
  count = 0;
  beg_time = time (NULL);
  for (int i=0; i<height; i++) {
    sprintf(curve[i], "                ");
    curve[i][size+16] = '\n';
  }
  curve[0][13] = '-';
  curve[0][14] = '>';
  sprintf(curve[height], "           0 -> ");
  for (int i=16; i<size+16; i++) curve[height][i] = '-';
  curve[height][size+16] = 0;
}

/* Destructor */
Histogram::~Histogram () {
  delete [] tab1;
  delete [] tab2;
}

/* A page is retrieved, add to stats */
void Histogram::pageHit (int x, int y) {
  tab1[end] += x;
  tab2[end] += y;
  if (tab1[end] > maxv) maxv = tab1[end];
  if (++count >= period) {
    count = 0;
    incrementEnd();
  }
}

/* Insert a new interval : */
void Histogram::incrementEnd () {
  end += 1;
  if (end >= size) end = 0;
  if (end <= beg) { /* have to delete the oldest interval */
    beg += 1;
    if (beg >= size) beg = 0;
    beg_time += period;
  }
  tab1[end] = 0;
  tab2[end] = 0;
}

/* Html output stat for last intervals */
void Histogram::write (int fds) {
  /* Compute the curve */
  int maxvbis = maxv;
  maxv = height; /* let's recompute it for next time */
  for (int i=beg, c=0; c<size; ++i, ++c) {
    if (i == size) i = 0; 
    if (tab1[i] > maxv) maxv = tab1[i];
    int h1 = (tab1[i] * height) / maxvbis;
    int h2 = (tab2[i] * height) / maxvbis;
    for (int j=0; j<height; ++j) {
      if (j >= h1)
        curve[height-1-j][c+16] = ' ';
      else if (j >= h2)
        curve[height-1-j][c+16] = 'x';
      else
        curve[height-1-j][c+16] = '*';
    }
  }

  /* Write the curve */
  ecrire (fds, "\n\nOne column is the number of pages retrieved during ");
  ecrireInt (fds, period);
  ecrire (fds, " seconds : \n");
  sprintf(curve[0], "%12d", maxvbis);
  curve[0][12] = ' ';
  int now_col = (global::now - beg_time) / period + 16;
  curve[height][now_col] = '|';
  ecrire(fds, curve[0]);
  curve[height][now_col] = '-';
  ecrireChar(fds, '\n');
  /* Show time bounds : */
  char *deb = asctime (localtime (&beg_time));
  deb[strlen(deb)-1] = 0;
  ecrire (fds, deb);
  snprintf(curve[height+1], size + 3 - strlen(deb), "%10000s", "");
  ecrire(fds, curve[height+1]);
  time_t fin_time = beg_time + period * size;
  char *fin = asctime (localtime (&fin_time));
  ecrire (fds, fin);
}

#endif // GRAPH
