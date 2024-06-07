#ifndef _PATHNAME_H_
#define _PATHNAME_H_

#include "unixfilesystem.h"

/**
 * Returns the inode number associated with the specified pathname.  This need only
 * handle absolute paths.  Returns a negative number (-1 is fine) if an error is 
 * encountered.
 */

/* No tenemos que chequear el caso sobre barra // */

/* 
/usr/bin/bash 
Te metes en usr, despues bin y y desp iteras todos los dirents hasta encontrar bash y ahi devolves el inumber
*/

int pathname_lookup(struct unixfilesystem *fs, const char *pathname);

#endif // _PATHNAME_H_
