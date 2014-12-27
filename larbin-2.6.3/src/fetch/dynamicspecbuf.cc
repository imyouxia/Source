// Larbin
// Sebastien Ailleret
// 10-12-01 -> 10-12-01

// use dynamic buffers when downloading specific pages

void initSpecific () { }

#define newSpec() ((void) 0)
#define endOfInput() 0

bool html::pipeSpec () {
  if (dynbuf == NULL) {
    if (pos > nearlyFullPage) {
      // need a dyn buf (big file)
      szDyn = 2*maxPageSize;
      dynbuf = new char[szDyn];
      nbSpec = buffer + pos - posParse;
      memcpy(dynbuf, posParse, nbSpec);
      dynbuf[nbSpec] = 0;
      pos = posParse - buffer;
    }
    return false;
  } else {
    int nb = buffer + pos - posParse;
    int newnb = nbSpec + nb;
    if (newnb >= maxSpecSize) {
      errno = tooBig;
      return true;
    }
    if (newnb >= szDyn) {
      // resize buffer
      szDyn *= 2;
      char *tmp = new char[szDyn];
      memcpy(tmp, dynbuf, nbSpec);
      delete[] dynbuf;
      dynbuf = tmp;
    }
    memcpy(dynbuf+nbSpec, posParse, nb);
    nbSpec = newnb;
    dynbuf[nbSpec] = 0;
    pos = posParse - buffer;
    return false;
  }
}

char * html::getContent() {
  if (dynbuf != NULL)
    return dynbuf;
  else
    return contentStart;
}

int html::getSize() {
  if (dynbuf != NULL) {
    return nbSpec;
  } else {
    return (buffer + pos - contentStart);
  }
}

#define constrSpec() dynbuf = NULL

#define destructSpec() if (dynbuf != NULL) { delete[] dynbuf; }
