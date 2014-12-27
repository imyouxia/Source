// Larbin
// Sebastien Ailleret
// 09-12-01 -> 09-12-01


static int nbdir = -1;
static int nbfile = filesPerDir;
static char *fileName;
static uint endFileName;
static int indexFds = -1;
static char buf[maxUrlSize+30];

/** give the name of the file given dir and file number
 * this char * is static */
static void getSpecName(int nbdir, int nbfile) {
  sprintf(fileName+endFileName, "d%5i/f%5i%s",
          nbdir, nbfile, privilegedExts[0]);
  for (int i=endFileName+1; fileName[i]==' '; i++)
    fileName[i] = '0';
  for (int i=endFileName+8; fileName[i]==' '; i++)
    fileName[i] = '0';
}

static void getIndexName(int nbdir) {
  sprintf(fileName+endFileName, "d%5i/index", nbdir);
  for (int i=endFileName+1; fileName[i]==' '; i++)
    fileName[i] = '0';
}

/** in case of specific save */
void initSpecific () {
  mkdir(specDir, S_IRWXU);
  endFileName = strlen(specDir);
  fileName = new char[endFileName+20];
  strcpy(fileName, specDir);
  if (fileName[endFileName-1] != '/') fileName[endFileName++] = '/';
}

/** open file descriptor */
void html::newSpec () {
  nbfile++;
  if (nbfile >= filesPerDir) { // new dir
    nbdir++; nbfile = 0;
    // create the directory
    getIndexName(nbdir);
    fileName[endFileName+6] = 0;
    if (mkdir(fileName, S_IRWXU) != 0) perror("trouble while creating dir");
    fileName[endFileName+6] = '/';
    // open new index
    close(indexFds);
    indexFds = creat(fileName, S_IRWXU);
    if (indexFds < 0) {
      cerr << "cannot open file " << fileName << " : "
           << strerror(errno) << endl;
      exit(1);
    }
  }
  mydir = nbdir;
  myfile = nbfile;
  // write index
  int s = sprintf(buf, "%4d  ", nbfile);
  s += here->writeUrl(buf+s);
  buf[s++] = '\n';
  ecrireBuff(indexFds, buf, s);
  // open new file
  getSpecName(nbdir, nbfile);
  fdsSpec = creat(fileName, S_IRWXU);
  if (fdsSpec < 0) {
    cerr << "cannot open file " << fileName << " : "
         << strerror(errno) << endl;
    exit(1);
  }
  nbSpec = 0;
}

/** feed file descriptor */
bool html::pipeSpec () {
  int nb = buffer + pos - posParse;
  nbSpec += nb;
  if (nbSpec >= maxSpecSize) {
    errno = tooBig;
    return true;
  }
  ecrireBuff(fdsSpec, posParse, nb);
  pos = posParse - buffer;
  return false;
}

/** get the content of the page */
char *html::getContent () {
  static char content[maxSpecSize];
  if (mydir >= 0) {
    getSpecName(mydir, myfile);
    int fds = open (fileName, O_RDONLY);
    if (fds < 0) perror(fileName);
    int cont = 1;
    int pos = 0;
    while (cont) {
      cont = read(fds, content+pos, maxSpecSize-1-pos);
      pos += cont;
    }
    content[pos] = 0;
    close(fds);
    return content;
  } else {
    return contentStart;
  }
}

#define getSize() nbSpec

#define constrSpec() \
  fdsSpec = -1; \
  mydir = -1;   // indicate no save

#define endOfInput() 0

#define destructSpec() if (fdsSpec != -1) close(fdsSpec);
