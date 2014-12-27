// Larbin
// Sebastien Ailleret
// 18-11-99 -> 21-05-01

#ifndef TEXT_H
#define TEXT_H

#include "utils/string.h"

/* lowercase a char */
char lowerCase (char a);

/* tests if b starts with a */
bool startWith (char *a, char *b);

/* test if b is forbidden by pattern a */
bool robotsMatch (char *a, char *b);

/* tests if b starts with a ignoring case */
bool startWithIgnoreCase (char *a, char *b);

/* test if b end with a */
bool endWith (char *a, char *b);

/* test if b end with a ignoring case
 * a can use min char, '.' (a[i] = a[i] | 32)
 */
bool endWithIgnoreCase (char *amin, char *b, int lb);

/* tests if b contains a */
bool caseContain (char *a, char *b);

/* create a copy of a string */
char *newString (char *arg);

/* Read a whole file
 */
char *readfile (int fds);

/* find the next token in the robots.txt, or in config file
 * must delete comments
 * no allocation (cf strtok); content is changed
 * param c is a bad hack for using the same function for robots and configfile
 */
char *nextToken(char **posParse, char c=' ');

/* does this char * match privilegedExt */
bool matchPrivExt(char *file);

/* does this char * match contentType */
bool matchContentType(char *ct);

#endif // TEXT_H
