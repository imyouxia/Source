#include "hashs.h"

unsigned int times33(char *str)
{
    unsigned int val = 0;
    while (*str) 
        val = (val << 5) + val + (*str++);
    return val;
}

unsigned int timesnum(char *str, int num)
{
    unsigned int val = 0;
    while (*str) 
        val = val * num + (*str++);
    return val;
}

unsigned int aphash(char *str)
{
    unsigned int val = 0;
    int i = 0;
    for (i = 0; *str; i++) 
        if ((i & 1) == 0)
            val ^= ((val << 7)^(*str++)^(val>>3));
        else
            val ^= (~((val << 11)^(*str++)^(val>>5)));

    return (val & 0x7FFFFFFF);	
}


unsigned int hash16777619(char *str)
{
    unsigned int val = 0;
    while (*str) {
        val *= 16777619;
        val ^= (unsigned int)(*str++);
    }
    return val;
}

unsigned int mysqlhash(char *str)
{
    register unsigned int nr = 1, nr2 = 4;
    while(*str) {
        nr ^= (((nr & 63) + nr2)*((unsigned int)*str++)) + (nr << 8);
        nr2 += 3;	
    }
    return (unsigned int)nr;
}


