/**
 * Network Testing tools
 *
 * ncopy - copy files 
 *  N:<->D: D:<->N: or N:<->N:
 *
 * Author: Thomas Cherryhomes
 *  <thom.cherryhomes@gmail.com>
 *
 * Released under GPL 3.0
 * See COPYING for details.
 */

#ifndef MISC_H
#define MISC_H

#include <stdbool.h>

/**
 * Print error passed by errnum
 */
void print_error(void);

/**
 * Detect wildcard
 * @param buf - the devicespec buffer
 * @return true if wildcard present, otherwise false.
 */
bool detect_wildcard(char* buf);

#endif /* MISC_H */
