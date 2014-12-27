// Larbin
// Sebastien Ailleret
// 27-08-01 -> 27-08-01

/* class hashTable
 * This class is in charge of making sure we don't crawl twice the same url
 */

#ifndef HASHDUP_H
#define HASHDUP_H

class hashDup {
 private:
  ssize_t size;
  char *table;
  char *file;

 public:
  /* constructor */
  hashDup (ssize_t size, char *init, bool scratch);

  /* destructor */
  ~hashDup ();

  /* set a page in the hashtable
   * return false if it was already there
   * return true if it was not (ie it is new)
   */
  bool testSet (char *doc);

  /* save in a file */
  void save ();
};

#endif // HASHDUP_H
