// Larbin
// Sebastien Ailleret
// 10-12-01 -> 10-12-01

// this file is included in the definition of class html (file.h)

  /* number of bits written */
  int nbSpec;
  /* feed file descriptor,
   * return true and set errno = tooBig if necessary */
  bool pipeSpec ();
  /* the dynamic buffer in which everything is saved */
  char *dynbuf;
  int szDyn;
  /** get the content of the page */
  char *getContent ();
  int getSize();
