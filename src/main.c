// Copyright 2020 "Bukhatov Bekzat"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

void freelist(char **arr) {
    int i = 0;
    while (arr[i]!= NULL) {
        free(arr[i]);
        i++;
    }
    free(arr[i]);
    free(arr);
}

char *get_word(char *end) {
    char *array = NULL;
    int index = 0;
    char c = getchar();
    do {
        array = realloc(array, (index + 1) * sizeof(char*));
        array[index] = c;
        index++;
        c = getchar();
    }while(c != ' ' && c != '\n');
    array = realloc(array, (index + 1) * sizeof(char*));
    array[index] = '\0';
    *end = c;
    return array;
}

char **get_list() {
    char **array = NULL;
    char end;
    int index = 0;
    do {
        array = realloc(array, (index + 1) * sizeof(char*));
        array[index] = get_word(&end);
        index++;
    } while (end != '\n');
    array = realloc(array, (index + 1) * sizeof(char*));
    array[index] = NULL;
    return array;
}

int main() {
    char **cmd = get_list();
    int fd, check;
    int i = 0;
    while ((strcmp(*cmd, "quit") != 0) && (strcmp(*cmd, "exit") != 0)) {
        check = 0;
        if (fork() > 0) {
            wait(NULL);
        } else {
            while (cmd[i] != NULL) {
                if ((strcmp(cmd[i], ">") == 0) && (cmd[i + 1] != NULL)) {
                    fd = open(cmd[i + 1], O_WRONLY | O_CREAT | O_TRUNC,
                         S_IRUSR | S_IWUSR);
                    check = 1;
                    break;
                }
                if ((strcmp(cmd[i], "<") == 0) && (cmd[i + 1] != NULL)) {
                    fd = open(cmd[i + 1], O_RDONLY);
                    check = 2;
                    break;
                }
                i++;
            }
            if (check == 1) {
                dup2(fd, 1);
                free(cmd[i + 1]);
                cmd[i] = NULL;
            } else if (check == 2) {
                dup2(fd, 0);
                free(cmd[i + 1]);
                cmd[i] = NULL;
            }

            if (execvp(cmd[0], cmd) < 0) {
                perror("exec failed");
                return 1;
            }
        }
        freelist(cmd);
        cmd = get_list();
        if (!check) {
            close(fd);
        }
    }
    freelist(cmd);
    return 0;
}
