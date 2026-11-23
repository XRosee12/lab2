#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s source destination\n",
                argv[0]);
        return 1;
    }

    int src_fd = open(argv[1], O_RDONLY);

    if (src_fd == -1) {
        perror("open source");
        return 1;
    }

    struct stat st;

    if (fstat(src_fd, &st) == -1) {
        perror("fstat");
        close(src_fd);
        return 1;
    }

    size_t size = (size_t)st.st_size;

    // 대상 파일 생성
    int dst_fd = open(argv[2],
                      O_RDWR | O_CREAT | O_EXCL, 0644);

    if (dst_fd == -1) {
        perror("open destination");
        close(src_fd);
        return 1;
    }

    if (ftruncate(dst_fd, st.st_size) == -1) {
        perror("ftruncate destination");
        close(src_fd);
        close(dst_fd);
        return 1;
    }

    // 빈 파일은 매핑하지 않고 종료
    if (size == 0) {
        printf("Empty file copied successfully.\n");
        close(src_fd);
        close(dst_fd);
        return 0;
    }

    // 원본 파일 메모리 매핑
    void *src_map = mmap(NULL, size, PROT_READ,
                         MAP_PRIVATE, src_fd, 0);

    if (src_map == MAP_FAILED) {
        perror("mmap source");
        close(src_fd);
        close(dst_fd);
        return 1;
    }

    // POSIX 공유 메모리 생성
    char shm_name[64];
    snprintf(shm_name, sizeof(shm_name),
             "/lab2_copy_%ld", (long)getpid());

    int shm_fd = shm_open(shm_name,
                         O_CREAT | O_EXCL | O_RDWR,
                         0600);

    if (shm_fd == -1) {
        perror("shm_open");
        munmap(src_map, size);
        close(src_fd);
        close(dst_fd);
        return 1;
    }

    if (ftruncate(shm_fd, st.st_size) == -1) {
        perror("ftruncate shared memory");
        shm_unlink(shm_name);
        close(shm_fd);
        munmap(src_map, size);
        close(src_fd);
        close(dst_fd);
        return 1;
    }

    void *shm_map = mmap(NULL, size,
                         PROT_READ | PROT_WRITE,
                         MAP_SHARED, shm_fd, 0);

    if (shm_map == MAP_FAILED) {
        perror("mmap shared memory");
        shm_unlink(shm_name);
        close(shm_fd);
        munmap(src_map, size);
        close(src_fd);
        close(dst_fd);
        return 1;
    }

    // 원본 파일 -> 공유 메모리
    memcpy(shm_map, src_map, size);

    // 대상 파일 메모리 매핑
    void *dst_map = mmap(NULL, size,
                         PROT_READ | PROT_WRITE,
                         MAP_SHARED, dst_fd, 0);

    if (dst_map == MAP_FAILED) {
        perror("mmap destination");
        munmap(shm_map, size);
        shm_unlink(shm_name);
        close(shm_fd);
        munmap(src_map, size);
        close(src_fd);
        close(dst_fd);
        return 1;
    }

    // 공유 메모리 -> 대상 파일
    memcpy(dst_map, shm_map, size);

    if (msync(dst_map, size, MS_SYNC) == -1) {
        perror("msync");
    }

    printf("File copied successfully!\n");
    printf("Source: %s\n", argv[1]);
    printf("Destination: %s\n", argv[2]);
    printf("Copied bytes: %zu\n", size);

    // 메모리 및 파일 정리
    munmap(src_map, size);
    munmap(shm_map, size);
    munmap(dst_map, size);

    close(src_fd);
    close(dst_fd);
    close(shm_fd);

    shm_unlink(shm_name);

    return 0;
}
	
