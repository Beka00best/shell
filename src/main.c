// Copyright 2020 "Bukhatov Bekzat"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

void freelist(char **arr, int *a, int count, int (*fd)[2]) {
    int i = 0;
    free(fd);
    for (int j = 0; j < count + 1; j++) {
        i = a[j];
        while (arr[i]!= NULL) {
            free(arr[i]);
            i++;
        }
    }
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
        return 1;
    }
    return 0;
}

int hasPipe(char **cmd, int **a) {
    int i = 0;
    int count = 0;
    char k;
    while (cmd[i] != NULL) {
        if ((strcmp(cmd[i], "|") == 0)) {
            free(cmd[i]);
            cmd[i] = NULL;
            count++;
            *a = realloc(*a, (count + 1) * sizeof(int));
            (*a)[count] = i + 1;
        }
        i++;
    }
    return count;
}

void Pipe(int x, int (*fd)[2], char **cmd, int *a) {
	int pid2;
	if (x == 0) {
        return;
    }
    for(int i = 1; i <= x; i++) {
        //fd = realloc(fd, (2 * sizeof(int *)) * i);
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
            Redirect(cmd, a[i]);
            OutFunction(cmd, a[i]);
        } else {
    	    // parent process
            close(fd[i - 1][1]);
            close(fd[i - 1][0]);
        }
    }
    close(fd[x][1]);
    close(fd[x][0]);
    for (int i = 0; i <= x; i++) {
        wait(NULL);
    }
}


int main() {
    char **cmd = get_list();
    int *a = NULL;
    a = malloc(1 * sizeof(int));
    a[0] = 0;
    //int fd[10][2];
    int (*fd)[2] = malloc(100 * sizeof(int[2]));
    int flag;
    pid_t pid;
    while ((strcmp(*cmd, "quit") != 0) && (strcmp(*cmd, "exit") != 0)) {
        if ((flag = hasPipe(cmd, &a))){
            for(int i = 0; i <= flag; i++) {
                pipe(fd[i]);
            }
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
            Pipe(flag, fd, cmd, a);
        }
        freelist(cmd, a, flag, fd);
        cmd = get_list();
    }
    freelist(cmd, a, flag, fd);
    return 0;
}
