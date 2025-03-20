/*
This a simple encrypt file an floder's program.
The projects by GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
Look at detail go to LICENSE file.

Time: 2025/03/20        Author: EbenezerDavid

*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <syslog.h>
#include "encryptff.h"

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    setsid();
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // 打开日志
    openlog("encrypt", LOG_PID, LOG_DAEMON);
    syslog(LOG_INFO, "File Protection Service started");
}

int main(int argc, char *argv[]) {
    // 检查是否以 systemd 服务方式运行
    int daemon = 0;
    if (argc == 1 && getenv("INVOCATION_ID") != NULL) {
        daemon = 1; // systemd 服务模式
    }

    if (daemon) {
        daemonize();
        // 守护进程模式：持续运行
        while (1) {
            syslog(LOG_INFO, "File Protection Service running...");
            sleep(60); // 每分钟记录一次日志
            // 确保不会退出
        }
        closelog();
        return EXIT_SUCCESS;
    }

    // 非守护进程模式：处理命令行参数或交互模式
    if (argc > 1) {
        if (strcmp(argv[1], "-l") == 0 && argc == 3) {
            char password[256];
            printf("Enter password to protect %s: ", argv[2]);
            if (scanf("%255s", password) != 1) {
                printf("Error: Failed to read password\n");
                return EXIT_FAILURE;
            }
            if (lock_file(argv[2], password) != 0) {
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        } else {
            printf("Usage: %s [-l <file_or_dir>]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }

    // 交互模式
    printf("File Protection Service\n");
    printf("Commands:\n");
    printf("  delete <file_or_dir> - Delete a protected file or directory\n");
    printf("  exit                - Exit the program\n");

    char command[256];
    while (1) {
        printf("> ");
        if (scanf("%255s", command) != 1) {
            printf("Error: Failed to read command\n");
            continue;
        }

        if (strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "delete") == 0) {
            char filepath[256];
            if (scanf("%255s", filepath) != 1) {
                printf("Error: Failed to read filepath\n");
                continue;
            }

            char password[256];
            printf("Enter password to delete %s: ", filepath);
            if (scanf("%255s", password) != 1) {
                printf("Error: Failed to read password\n");
                continue;
            }

            if (unlock_and_delete(filepath, password) != 0) {
                printf("Failed to delete %s\n", filepath);
            }
        } else {
            printf("Unknown command. Available commands: delete <file_or_dir>, exit\n");
        }
    }

    return EXIT_SUCCESS;
}