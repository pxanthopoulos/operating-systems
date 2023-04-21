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

	//if no infiles & no outfiles, create empty outfile with default name fconc2.out 
	if (argc == 1) {
		int oflags = O_CREAT | O_WRONLY | O_TRUNC;
		int mode = S_IRUSR | S_IWUSR;
		int fd = open("fconc2.out", oflags, mode);
		close(fd);
		return 0;
	}

	//if zero infiles & 1 outfile, create empty outfile with given name
	if (argc == 1) {
		int oflags = O_CREAT | O_WRONLY | O_TRUNC;
		int mode = S_IRUSR | S_IWUSR;
		int fd = open(argv[1], oflags, mode);
		close(fd);
		return 0;
	}

	//if one or more infiles, ...	

	//first check if outfile is same as any infile, this is forbidden
	int no_in = argc-2; //number of infiles

	for (int i=0; i<no_in; i++) {
		if (strcmp(argv[i], argv[argc-1]) == 0) {
			fprintf(stderr, "Output file cannot be the same as any input file!\n");
			fprintf(stderr, "Usage: ./fconc2 infile1 infile2 ... outfile\n");
			exit(1);
		}
	}

	char buff[1];
	int len = sizeof(buff);

	int fd[argc-1];

	//open destination file
	int oflags = O_CREAT | O_WRONLY | O_TRUNC;
	int mode = S_IRUSR | S_IWUSR;
	fd[argc-2] = open(argv[argc-1], oflags, mode);
	if (fd[argc-2] == -1) {
		perror("Error in file open (destination file).\n");
		return 1;
	}

	//read-write at the same time
	for (int i=0; i<no_in; ++i) {

		fd[i] = open(argv[i+1], O_RDONLY, S_IRUSR);
		if (fd[i] == -1) {
			fprintf(stderr, "%s: No such file or directory\n", argv[i+1]);
			exit(1);
		}

		int count1 = 0;
		int count2 = 0;

		do {
			count1 = readfile(buff, fd[i], len);
			if (count1 == -1) return 1;
			count2 = writefile(buff, fd[argc-2], count1);
			if (count2 == -1) return 1;
		}
		while (count1 > 0);

		close(fd[i]);
	}

	close(fd[argc-2]);

	return 0;

}