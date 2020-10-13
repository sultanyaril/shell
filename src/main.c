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

void check_files(char **cmd, int *inp, int *out, int start_point) {
    char *search = cmd[0];
    int flag = 0;
    int fd = 0;
    for (int i = start_point; cmd[i + 1] != NULL ;) {
        if (search[0] == '<') {
            fd = open(cmd[i + 1], O_RDONLY | O_CREAT, 0);
            flag = 0;
            free(cmd[i+1]);
            cmd[i] = NULL;
            break;
        } else {
            if (search[0] == '>') {
                fd = open(cmd[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0);
                flag = 1;
                free(cmd[i + 1]);
                cmd[i] = NULL;
                break;
            }
        }
        search = cmd[++i];
    }
    if (flag)
        *out = fd;
    else
        *inp = fd;
}

int *segment_line(char **cmd, int *inp, int *out) {
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
    check_files(cmd, inp, out, 0);
    if (cnt != 1)
        check_files(cmd, inp, out, answ[cnt-1]);
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
        int input_fd = 0, output_fd = 1;
        int *seg_num = segment_line(cmd, &input_fd, &output_fd);
        int sizefd = 1;
        int (*fd)[2] = malloc(sizefd * sizeof(int[2]));
        fd[sizefd - 1][0] = input_fd;
        for (int j = 1; seg_num[j]; j++) {
            sizefd++;
            fd = realloc(fd, sizefd * sizeof(int[2]));
            int pipefd[2];
            pipe(pipefd);
            fd[sizefd - 2][1] = pipefd[1];
            fd[sizefd - 1][0] = pipefd[0];
        }
        fd[sizefd - 1][1] = output_fd;
        int i = 0;
        do {
            if (fork() == 0) {
                dup2(fd[i][0], 0);
                dup2(fd[i][1], 1);
                if (execvp(cmd[seg_num[i]], cmd + seg_num[i]) < 0)
                    perror("exec error");
                close(fd[i][0]);
                close(fd[i][1]);
                return 1;
            } else {
                wait(NULL);
            }
            i++;
        } while (seg_num[i]);
        free_list(cmd, seg_num);
        if (input_fd != 0)
            close(input_fd);
        if (output_fd != 1)
            close(output_fd);
        cmd = get_list();
    }
    free(cmd[0]);
    free(cmd);
    return 0;
}
