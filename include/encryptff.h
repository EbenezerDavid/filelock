/*
This a simple encrypt file an floder's program.
The projects by GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
Look at detail go to LICENSE file.

Time: 2025/03/20        Author: EbenezerDavid

*/



#ifndef ENCRYPTFF_H
#define ENCRYPTFF_H

#include <sys/stat.h>
#include <sys/types.h>

int lock_file(const char *path, const char *password);
int unlock_and_delete(const char *path, const char *password);

#endif