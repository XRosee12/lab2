#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

volatile sig_atomic_t signal_received = 0;

void signal_handler(int signo) {
    if (signo == SIGUSR1) {
        signal_received = 1;
    }
}

int main(void) {
    struct sigaction sa = {0};

    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // 자식 프로세스
        printf("[Child] PID: %ld\n", (long)getpid());
        printf("[Child] Waiting for SIGUSR1...\n");
        fflush(stdout);

        while (!signal_received) {
            pause();
        }

        printf("[Child] SIGUSR1 received!\n");
        printf("[Child] Exiting...\n");
        return 0;
    }

    // 부모 프로세스
    printf("[Parent] PID: %ld\n", (long)getpid());
    printf("[Parent] Child PID: %ld\n", (long)pid);
    fflush(stdout);

    sleep(2);

    printf("[Parent] Sending SIGUSR1...\n");
    fflush(stdout);

    if (kill(pid, SIGUSR1) == -1) {
        perror("kill");
        return 1;
    }

    waitpid(pid, NULL, 0);

    printf("[Parent] Child process finished.\n");

    return 0;
}
