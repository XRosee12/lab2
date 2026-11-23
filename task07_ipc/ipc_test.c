#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int fd[2];
    pid_t pid;
    char buffer[100] = {0};

    const char *message = "Hello from parent process!";

    // 파이프 생성
    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    // 자식 프로세스 생성
    pid = fork();

    if (pid < 0) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    if (pid == 0) {
        // 자식 프로세스: 데이터 읽기
        close(fd[1]);

        ssize_t n = read(fd[0], buffer, sizeof(buffer) - 1);

        if (n < 0) {
            perror("read");
            close(fd[0]);
            return 1;
        }

        buffer[n] = '\0';

        printf("[Child] Received: %s\n", buffer);

        close(fd[0]);
        return 0;
    }

    // 부모 프로세스: 데이터 쓰기
    close(fd[0]);

    printf("[Parent] Sending: %s\n", message);
    fflush(stdout);

    if (write(fd[1], message, strlen(message)) == -1) {
        perror("write");
        close(fd[1]);
        waitpid(pid, NULL, 0);
        return 1;
    }

    close(fd[1]);

    // 자식 프로세스 종료 대기
    waitpid(pid, NULL, 0);

    printf("[Parent] Child process finished.\n");

    return 0;
}
