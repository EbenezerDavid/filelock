/*
This a simple encrypt file an floder's program.
The projects by GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
Look at detail go to LICENSE file.

Time: 2025/03/20        Author: EbenezerDavid

*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <libgen.h>
#include <dirent.h>
#include <unistd.h>
#include "encryptff.h"

#define PROTECTED_FILE "protected_files.txt"

int lock_file(const char *path, const char *password) {
    struct stat path_stat;
    if (lstat(path, &path_stat) != 0) {
        printf("Error: Cannot stat %s\n", path);
        return 1;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        DIR *dir = opendir(path);
        if (dir == NULL) {
            perror("opendir");
            return 1;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char subpath[512];
            snprintf(subpath, sizeof(subpath), "%s/%s", path, entry->d_name);
            lock_file(subpath, password);
        }
        closedir(dir);
        return 0;
    }

    // 存储 inode、路径和密码
    FILE *outfile = fopen(PROTECTED_FILE, "a+");
    if (outfile == NULL) {
        printf("Error: Cannot create file %s\n", PROTECTED_FILE);
        return 1;
    }
    fprintf(outfile, "%lu %s %s\n", (unsigned long)path_stat.st_ino, path, password);
    fclose(outfile);

    // 设置文件不可变
    char command[256];
    snprintf(command, sizeof(command), "chattr +i %s", path);
    if (system(command) != 0) {
        perror("chattr +i");
        return 1;
    }
    printf("Locked file: %s (inode: %lu)\n", path, (unsigned long)path_stat.st_ino);
    return 0;
}

int unlock_and_delete(const char *path, const char *password) {
    struct stat path_stat;
    if (lstat(path, &path_stat) != 0) {
        printf("Error: Cannot stat %s\n", path);
        return 1;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        DIR *dir = opendir(path);
        if (dir == NULL) {
            perror("opendir");
            return 1;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char subpath[512];
            snprintf(subpath, sizeof(subpath), "%s/%s", path, entry->d_name);
            unlock_and_delete(subpath, password);
        }
        closedir(dir);
        return 0;
    }

    FILE *infile = fopen(PROTECTED_FILE, "r");
    if (infile == NULL) {
        printf("Error: Cannot read password file %s\n", PROTECTED_FILE);
        return 1;
    }

    unsigned long saved_inode;
    char saved_path[256];
    char saved_password[256];
    int found = 0;

    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        fclose(infile);
        return 1;
    }

    while (fscanf(infile, "%lu %s %s\n", &saved_inode, saved_path, saved_password) == 3) {
        if (strcmp(saved_path, path) == 0) {
            if (strcmp(password, saved_password) != 0) {
                printf("Wrong password for %s\n", path);
                fprintf(temp, "%lu %s %s\n", saved_inode, saved_path, saved_password);
                found = 1;
            } else {
                found = 1;
                continue;
            }
        } else {
            fprintf(temp, "%lu %s %s\n", saved_inode, saved_path, saved_password);
        }
    }
    fclose(infile);
    fclose(temp);

    if (!found) {
        printf("File %s not found in protected list\n", path);
        remove("temp.txt");
        return 1;
    }

    // 如果文件为空，删除它
    FILE *check = fopen("temp.txt", "r");
    fseek(check, 0, SEEK_END);
    if (ftell(check) == 0) {
        fclose(check);
        remove(PROTECTED_FILE);
        remove("temp.txt");
    } else {
        fclose(check);
        rename("temp.txt", PROTECTED_FILE);
    }

    if (strcmp(password, saved_password) == 0) {
        char command[256];
        snprintf(command, sizeof(command), "chattr -i %s", path);
        if (system(command) != 0) {
            perror("chattr -i");
            return 1;
        }
        if (unlink(path) != 0) {
            perror("unlink");
            return 1;
        }
        printf("File %s deleted successfully.\n", path);
    }
    return 0;
}