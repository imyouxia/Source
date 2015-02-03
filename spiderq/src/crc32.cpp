#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "crc32.h"

static unsigned int   CRC32[256];
static char   init = 0;

//初始化表
static void init_table()
{
    int   i,j;
    unsigned int   crc;
    for(i = 0;i < 256;i++) {
        crc = i;
        for(j = 0;j < 8;j++) {
            if(crc & 1)
                crc = (crc >> 1) ^ 0xEDB88320;
            else
                crc = crc >> 1;
        }
        CRC32[i] = crc;
    }
}

//crc32实现函数
unsigned int crc32( unsigned char *buf, int len)
{
    unsigned int ret = 0xFFFFFFFF;
    int   i;
    if( !init ) {
        init_table();
        init = 1;
    }
    for(i = 0; i < len;i++)
        ret = CRC32[((ret & 0xFF) ^ buf[i])] ^ (ret >> 8);

    return ~ret;
}

/*
   int main(int argc, char *argv[])
   {
   if (argc < 2) {  
   fprintf (stderr, "usage: crc32 string\n");  
   exit (1);  
   }  
   printf("%s: %u\n", argv[1], crc32((unsigned char *)argv[1], strlen(argv[1])));
   return 0;
   }
   */
