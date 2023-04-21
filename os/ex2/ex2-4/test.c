#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

//int num = 3;

void child(int in, int out, int num) {
    close(in);
    write(out, &num, sizeof(int));
}

int main(int argc, char *argv[]) {
	pid_t p;
	int pfd[2];
	int status;
	int result[2];
	
	if (pipe(pfd) < 0) {
		perror("pipe");
		exit(1);
	}
	
    for (int i = 0; i < 5; i++) {
        p = fork();
        if (p < 0) {
            /* fork failed */
            perror("fork");
            exit(1);
        }
        if (p == 0) {
	        /* In child process */
		    child(pfd[0], pfd[1], i);
            exit(0);
        }
        wait(&status);
	} 
    
    close(pfd[1]);

	for (int i = 0; i < 2; i++) {
        read(pfd[0], &result[i], sizeof(int));
    }
	
    printf("%d  %d\n", result[0], result[1]);

	return 0;
}