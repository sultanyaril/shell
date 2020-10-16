#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
char *get_word(char *end) {
    char *answ = NULL;
    char c = getchar();
    int cnt = 0;
    while (c != '\n' && c != ' ') {
        cnt++;
        answ = realloc(answ, (cnt + 1) * sizeof(char));
        answ[cnt - 1] = c;
        c = getchar();
    }
    answ[cnt] = 0;
    *end = c;
    return answ;
}

char **get_list() {
    char **answ = NULL;
    int cnt = 0;
    char end = 0;
    char *c;
    while (end != '\n') {
        c = get_word(&end);
        cnt++;
        answ = realloc(answ, (cnt + 1) * sizeof(char *));
        answ[cnt - 1] = c;
    }
    answ[cnt] = NULL;
    return answ;
}

int check_files(char **cmd, int startpoint) {
    char *search = cmd[0];
    int flag = 0;  // 0-search first, 1-found 1, 2-found 2, 3-search sec
    for (int i = startpoint; cmd[i + 1] != NULL ;) {
        if (strcmp(search, "<") == 0) {
            if (flag) {
                int fd = open(cmd[i + 1],
                    O_RDONLY | O_CREAT,
                    0);
                dup2(fd, 0);
                close(fd);
                free(cmd[i + 1]);
                cmd[i] = NULL;
                if (flag == 3) {
                    i = startpoint - 1;
                    flag = 2;
                }
            } else {
                flag = 3;
            }
        }
        if (strcmp(search, ">") == 0) {
            if (flag) {
                int fd = open(cmd[i + 1],
                    O_WRONLY | O_CREAT | O_TRUNC,
                    S_IRUSR | S_IWUSR);
                dup2(fd, 1);
                close(fd);
                free(cmd[i + 1]);
                cmd[i] = NULL;
                if (flag == 3) {
                    i = startpoint - 1;
                    flag = 2;
                }
            } else {
                flag = 3;
            }
        }
        search = cmd[++i];
        if (flag == 3 && cmd[i + 1] == NULL) {
            i = startpoint;
            flag = 1;
        }
    }
    return flag;
}

int *segment_line(char **cmd) {
    int cnt = 1;
    int *answ = malloc((cnt + 1) * sizeof(int));  // why realloc isnt working?
    answ[0] = 0;
    for (int i = 0; cmd[i]; i++) {
        if (strcmp(cmd[i], "|") == 0) {
            cnt++;
            answ = realloc(answ, (cnt + 1) * sizeof(int));
            answ[cnt - 1] = i + 1;
            free(cmd[i]);
            cmd[i] = NULL;
        }
    }
    answ[cnt] = 0;
    return answ;
}

void free_list(char **arr, int *numb) {
    int j = 0;
    do {
        for (int i = numb[j]; arr[i]; i++) {
            free(arr[i]);
        }
        j++;
    } while (numb[j]);
    free(numb);
    free(arr);
}

int main() {
    char **cmd = get_list();
    while (strcmp(cmd[0], "exit") && strcmp(cmd[0], "quit")) {
        int *seg_num = segment_line(cmd);
        int (*fd)[2] = NULL;
        int segsize = 1;
        for ( ; seg_num[segsize] ; segsize++) {
            fd = realloc(fd, segsize * sizeof(int[2]));
        }
        for (int i = 0 ; i < segsize; i++) {
            if ((i + 1) != segsize) {
                pipe(fd[i]);
            }
            if (fork() == 0) {
                if (i == 0) {
                    if (check_files(cmd, 0) == 2 && seg_num[i + 1]) {
                        execvp(cmd[0], cmd);
                        }
                } else {
                    dup2(fd[i - 1][0], 0);
                    close(fd[i - 1][0]);
                    close(fd[i - 1][1]);
                }
                if (seg_num[i + 1] == 0) {
                    check_files(cmd, seg_num[i]);
                } else {
                    dup2(fd[i][1], 1);
                    close(fd[i][0]);
                    close(fd[i][1]);
                }
                if (execvp(cmd[seg_num[i]], cmd + seg_num[i]) < 0)
                    perror("exec error");
                return 1;
            } else {
                if (i != 0 && fd) {
                    close(fd[i - 1][0]);
                    close(fd[i - 1][1]);
                }
                wait(NULL);
            }
        }
        if (fd) {
            close(fd[segsize - 2][0]);
            close(fd[segsize - 2][1]);
            free(fd);
        }
        free_list(cmd, seg_num);
        cmd = get_list();
    }
    free(cmd[0]);
    free(cmd);
    return 0;
}
