#ifndef SHA1_H
#define SHA1_H

typedef unsigned int u32;

typedef struct {
    u32  h0,h1,h2,h3,h4;
    u32  nblocks;
    unsigned char buf[64];
    int  count;
} SHA1_CONTEXT;


extern void sha1_init(SHA1_CONTEXT *hd);
extern void sha1_write(SHA1_CONTEXT *hd, unsigned char *inbuf, size_t inlen);
extern void sha1_final(SHA1_CONTEXT *hd);

#endif
