#ifndef ENCRYPTFF_H
#define ENCRYPTFF_H

#include <sys/stat.h>
#include <sys/types.h>

int lock_file(const char *path, const char *password);
int unlock_and_delete(const char *path, const char *password);

#endif