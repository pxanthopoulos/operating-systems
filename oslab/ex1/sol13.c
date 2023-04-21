#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    int fd = openat(AT_FDCWD, ".hello_there", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    pid_t pid = fork();

    if (pid == 0) {
        char *const args[] = { NULL };
        char *const env[] = { NULL };
        execve("./riddle", args, env);
    }
    else {
        sleep(5);
        ftruncate(fd, 32768);
        wait(NULL);
    }
}