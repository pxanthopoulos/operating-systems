#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int readfile(char *buffer, int fd, int x) {
	ssize_t rcnt;
	int idx = 0;
	do {
		rcnt = read(fd, buffer+idx, x);
		if (rcnt == 0) {
			return idx;
		}
		if (rcnt == -1) {
			perror("Error in file read.\n");
			return -1;	
		}
		idx += rcnt;
		x-=rcnt;
	}
	while (x > 0);
	return idx;
}

int writefile(const char *buffer, int fd, int x) {
	ssize_t wcnt;
	int idx = 0;
	do {
		wcnt = write(fd, buffer+idx, x);
		if (wcnt == 0) {
			return idx;
		}
		if (wcnt == -1){
			perror("Error in file write.\n");
			return -1;
		}
		idx += wcnt;
		x-=wcnt;
	}
	while (x > 0);
	return idx;
}


int main(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "Usage: ./fconc infile1 infile2 [outfile (default:fconc.out)]\n");
		exit(1);
	}

	int fd[3];

	fd[0] = open(argv[1], O_RDONLY, S_IRUSR);
	if (fd[0] == -1) {
		fprintf(stderr, "%s: No such file or directory\n", argv[1]);
		exit(1);
	}

	fd[1] = open(argv[2], O_RDONLY, S_IRUSR);
	if (fd[1] == -1) {
		fprintf(stderr, "%s: No such file or directory\n", argv[2]);
		exit(1);
	}

	if (argc == 4) {
		if (strcmp(argv[1], argv[3]) == 0 || strcmp(argv[2], argv[3]) == 0) {
			fprintf(stderr, "Output file cannot be the same as input file!\n");
			exit(1);
		}

		int oflags = O_CREAT | O_WRONLY | O_TRUNC;
		int mode = S_IRUSR | S_IWUSR;
		fd[2] = open(argv[3], oflags, mode);
		if (fd[2] == -1) {
			perror("Error in file open (destination file).\n");
			return 1;
		}
	}

	else {
		int oflags = O_CREAT | O_WRONLY | O_TRUNC;
		int mode = S_IRUSR | S_IWUSR;
		fd[2] = open("fconc.out", oflags, mode);
		if (fd[2] == -1) {
			perror("Error in file open (destination file).\n");
			return 1;
		}
	}
	
	char buff[1024];
	int len = sizeof(buff);

	int count1=0;
	int count2=0;
	int count=0;
	
	do {
		count1 = readfile(buff, fd[0], len);
		if (count1 == -1) return 1;
		count = writefile(buff, fd[2], count1);
		if (count == -1) return 1;
	}
	while (count1 > 0);
	
	do {
                count2 = readfile(buff, fd[1], len);
                if (count2 == -1) return 1;
                count = writefile(buff, fd[2], count2);
                if (count == -1) return 1;
        }
        while (count2 > 0);
	
	close(fd[0]);
	close(fd[1]);
	close(fd[2]);

	return 0;
}