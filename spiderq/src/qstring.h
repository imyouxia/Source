#ifndef QSTRING_H
#define QSTRING_H

/* Concat multi strings into one.
 * argc: the number of strings
 */
extern char * strcat2(int argc, const char *str1, const char * str2, ...);

/* Trim the string. This function will NOT allocate new memory */
extern char * strim(char *str);

/* Split string.
 * count: size of splitted strings
 * limit: how many times to split string 
 */
extern char ** strsplit(char *line, char delimeter, int *count, int limit);

/* Turn 'yes' or 'no' to integer, or -1 if error 
 * Case insensitive!
 */
extern int yesnotoi(char *str);

#endif
