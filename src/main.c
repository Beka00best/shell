// Copyright 2020 "Bukhatov Bekzat"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define ANSI_COLOR_BLUE    "\x1b[1;34m"
#define ANSI_COLOR_RESET   "\x1b[0m"

pid_t pid = 1;

void freelist(char **arr, int *positionCommands, int count) {
    int i = 0;
    for (int j = 0; j < count + 1; j++) {
        i = positionCommands[j];
        while (arr[i]!= NULL) {
            free(arr[i]);
            i++;
        }
    }
    free(arr);
    free(positionCommands);
}

char *get_word(char *end) {
    char *array = NULL;
    int index = 0;
    if (*end == '\n') {
        return NULL;
    }
    char c = getchar();
    if (c == '\n') {
        *end = c;
        return NULL;
    }
    while (index == 0 && (c == ' ' || c == '\t' )) { 
        c = getchar();
        if (c == '\n') {
            *end = c;
            return NULL;
        }
    }
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
        //
        index++;
    } while (end != '\n');
    array = realloc(array, (index + 1) * sizeof(char*));
    array[index] = NULL;
    return array;
}

void dup2Action(char **cmd, int file, int x, int index) {
    dup2(file, x);
    close(file);
    free(cmd[index]);
    cmd[index] = NULL;
    free(cmd[index + 1]);
    cmd[index + 1] = NULL;
    return;
}

int bgSearch(char **cmd, char *word) {
    int pos = 0;
    for (pos = 0; cmd[pos] != NULL; pos++) {
        if (strcmp(word, cmd[pos]) == 0) {
            free(cmd[pos]);
            cmd[pos] = NULL;
            return pos;
        }
    }
    return 0;
}

int Checking(char **cmd, int x, int flagCD) {
    int i = x;
    int input, output;
    while (cmd[i] != NULL) {
        if ((strcmp(cmd[i], ">") == 0) && (cmd[i + 1] != NULL)) {
            input = open(cmd[i + 1], O_WRONLY | O_CREAT | O_TRUNC,
                 S_IRUSR | S_IWUSR);
            dup2Action(cmd, input, 1, i);
            break;
        }
        if ((strcmp(cmd[i], "<") == 0) && (cmd[i + 1] != NULL)) {
            output = open(cmd[i + 1], O_RDONLY);
            dup2Action(cmd, output, 0, i);
            break;
        }
        if ((strcmp(cmd[i], "cd") == 0)) {
            flagCD = 2;
            if ((cmd[i + 1] == NULL) || (strcmp(cmd[i + 1], "~") == 0)) {
                chdir(getenv("HOME"));
            } else {
                chdir(cmd[i + 1]);
            }
            break;
            // char *newCWD[100];
            // getcwd(newCWD, sizeof(newCWD));
            // setenv("PWD", newCWD, 1);
        }
        i++;
    }
    return flagCD;
}

int OutFunction(char **cmd, int position) {
    if (execvp(cmd[position], cmd + position) < 0) {
        perror("exec failed");
        exit(1);
    }
    return 0;
}

int hasPipe(char **cmd, int **positionCommands) {
    int i = 0;
    int count = 0;
    while (cmd[i] != NULL) {
        if ((strcmp(cmd[i], "|") == 0)) {
            free(cmd[i]);
            cmd[i] = NULL;
            count++;
            *positionCommands = realloc(*positionCommands,
                (count + 1) * sizeof(int));
            (*positionCommands)[count] = i + 1;
        }
        i++;
    }
    return count;
}

void Pipe(int x, int (*fd)[2], char **cmd, int *positionCommands, int flagCD) {
    int pid2;
    for (int i = 1; i <= x; i++) {
        if ((pid2 = fork()) == 0) {
            // sub process
            dup2(fd[i - 1][0], 0);
            close(fd[i - 1][0]);
            close(fd[i - 1][1]);
            if (i != x) {
                dup2(fd[i][1], 1);
            }
            for (int k = 0; k <= x; k++) {
                close(fd[k][1]);
                close(fd[k][0]);
            }
            Checking(cmd, positionCommands[i], flagCD);
            OutFunction(cmd, positionCommands[i]);
        } else {
            // parent process
            close(fd[i - 1][1]);
            close(fd[i - 1][0]);
        }
    }
}

void showPWD() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf(ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET "$ ", cwd);
    } else {
        perror("getcwd failed\n");
    }
}

void handler(int signo) {
    puts("\nRecieved SIGINT");
    if (pid != 1) {
        kill(pid, SIGINT);
    }
}

int main() {
    showPWD();
    char **cmd = get_list();
    int *positionCommands;
    positionCommands = malloc(1 * sizeof(int));
    positionCommands[0] = 0;
    int flag = 0, bg, flagCD = 0;
    int wstatus;
    signal(SIGINT, handler);
    while ((strcmp(*cmd, "quit") != 0) && (strcmp(*cmd, "exit") != 0)) {
        bg = bgSearch(cmd, "&");
        flag = hasPipe(cmd, &positionCommands);
        int (*fd)[2] = malloc((flag + 1) * sizeof(int[2]));
        for (int i = 0; i <= flag; i++) {
            pipe(fd[i]);
        }
        if ((pid = fork()) == 0) {
            if (flag) {
                dup2(fd[0][1], 1);
            }
            for (int k = 0; k <= flag; k++) {
                close(fd[k][1]);
                close(fd[k][0]);
            }
            flagCD = Checking(cmd, positionCommands[0], flagCD);
            if (!flagCD) {
                OutFunction(cmd, positionCommands[0]);
            }
        } else if (pid > 0) {
            Pipe(flag, fd, cmd, positionCommands, flagCD);
            if (bg == 0) {
                waitpid(pid, &wstatus, 0);
            }
        }
        close(fd[flag][1]);
        close(fd[flag][0]);
        if (bg == 0) {
            for (int i = 0; i <= flag; i++) {
                waitpid(pid, NULL, 0);
            }
        }
        free(fd);
        freelist(cmd, positionCommands, flag);
        positionCommands = malloc(1 * sizeof(int));
        positionCommands[0] = 0;
        showPWD();
        cmd = get_list();
    }
    freelist(cmd, positionCommands, 0);
    return 0;
}
