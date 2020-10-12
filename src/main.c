// Copyright 2020 "Bukhatov Bekzat"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

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

void dup2Action(char **cmd, int file, int x, int index) {
    dup2(file, x);
    close(file);
    free(cmd[index]);
    cmd[index] = NULL;
    free(cmd[index + 1]);
    cmd[index + 1] = NULL;
    return;
}

int Redirect(char **cmd, int x) {
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
        i++;
    }
    return i;
}

int OutFunction(char **cmd, int x) {
    if (execvp(cmd[x], cmd + x) < 0) {
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
            *positionCommands = realloc(*positionCommands, (count + 1) * sizeof(int));
            (*positionCommands)[count] = i + 1;
        }
        i++;
    }
    return count;
}

void Pipe(int x, int (*fd)[2], char **cmd, int *positionCommands) {
	int pid2;
    for(int i = 1; i <= x; i++) {
        if ((pid2 = fork()) == 0) {
            // sub process
            dup2(fd[i - 1][0], 0);
            close(fd[i - 1][0]);
            close(fd[i - 1][1]);
            if (i != x) {
                dup2(fd[i][1], 1);
            }
            close(fd[i][1]);
            close(fd[i][0]);
            Redirect(cmd, positionCommands[i]);
            OutFunction(cmd, positionCommands[i]);
        } else {
    	    // parent process
            close(fd[i - 1][1]);
            close(fd[i - 1][0]);
        }
    }
}


int main() {
    char **cmd = get_list();
    int *positionCommands;
    positionCommands = malloc(1 * sizeof(int));
    positionCommands[0] = 0;
    int flag = 0;
    pid_t pid;
    while ((strcmp(*cmd, "quit") != 0) && (strcmp(*cmd, "exit") != 0)) {
        flag = hasPipe(cmd, &positionCommands);
        int (*fd)[2] = malloc((flag + 1) * sizeof(int[2]));
        for(int i = 0; i <= flag; i++) {
            pipe(fd[i]);
        }
        if ((pid = fork()) == 0) {
            if (flag) {
                dup2(fd[0][1], 1);
            }
            close(fd[0][1]);
            close(fd[0][0]);
            Redirect(cmd, a[0]);
            OutFunction(cmd, a[0]);
        } else if (pid > 0) {
            Pipe(flag, fd, cmd, positionCommands);
        }
        close(fd[flag][1]);
        close(fd[flag][0]);
        for (int i = 0; i <= flag; i++) {
            wait(NULL);
        }
        free(fd);
        freelist(cmd, positionCommands, flag);
        positionCommands = malloc(1 * sizeof(int));
        positionCommands[0] = 0;
        cmd = get_list();
    }
    freelist(cmd, positionCommands, 0);
    return 0;
}
