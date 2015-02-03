#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H

#include <unistd.h>
#include <string.h>

/* search if this url has been crawled */
extern int search(char *url);

#endif
