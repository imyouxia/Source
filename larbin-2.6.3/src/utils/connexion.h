// Larbin
// Sebastien Ailleret
// 15-11-99 -> 14-12-99

#ifndef CONNEXION_H
#define CONNEXION_H

/* make write until everything is written
 * return 0 on success, 1 otherwise
 * Don't work on non-blocking fds...
 */
int ecrire (int fd, char *buf);

/* make write until everything is written
 * return 0 on success, 1 otherwise
 * Don't work on non-blocking fds...
 */
int ecrireBuff (int fd, char *buf, int count);

/** Write an int on a fds
 * (uses ecrire)
 */
int ecrireInt (int fd, int i);
int ecrireInt2 (int fd, int i);
int ecrireInti (int fd, int i, char *f);
int ecrireIntl (int fd, long i, char *f);

/** Write an int on a fds
 * (uses ecrire)
 */
int ecrireLong (int fd, long i);

/* Write a char on a fds
 * return 0 on success, 1 otherwise
 * Don't work on non-blocking fds...
 */
int ecrireChar (int fd, char c);


#endif // CONNEXION_H
