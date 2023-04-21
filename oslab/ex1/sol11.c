#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
    int fd1, fd2;
    char addr[4096];
    fd1 = open("secret_number", O_RDWR);
    fd2 = open("helper_file", O_CREAT|O_RDWR );
    sleep(10);
    read(fd1, addr, 4096);
    write(fd2, addr, 4096);
    close(fd2);
    close(fd1);
}