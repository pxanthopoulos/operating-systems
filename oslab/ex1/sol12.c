#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

int main(int argc, char *argv[]) {
    int fd = open(argv[1], O_RDWR);
    char *buff = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    buff[111] = argv[2][0];
}
