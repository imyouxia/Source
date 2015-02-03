#ifndef HASHS_H
#define HASHS_H

extern unsigned int times33(char *str);
extern unsigned int timesnum(char *str, int num);
extern unsigned int aphash(char *str);
extern unsigned int hash16777619(char *str);
extern unsigned int mysqlhash(char *str);

#endif
