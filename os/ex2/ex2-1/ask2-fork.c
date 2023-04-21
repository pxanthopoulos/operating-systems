#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "proc-common.h"

#define SLEEP_PROC_SEC  10
#define SLEEP_TREE_SEC  3

void fork_procs(char *name, int rec_status) {
	change_pname(name);
	printf("%s: Sleeping...\n",name);
	sleep(SLEEP_PROC_SEC);
	printf("%s: Exiting...\n", name);
	exit(rec_status);
}

int main(void) {
	pid_t pid_A;
	int status_A;

	/* Fork root of process tree */
	pid_A = fork();

	if (pid_A < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid_A == 0) {
		//i am A

		change_pname("A");
		
		pid_t pid_B, pid_C;
		int status_B, status_C;
		
		pid_B = fork();
		if (pid_B < 0) {
			perror("main: fork");
			exit(1);
		}
		if (pid_B == 0) {
			// i am B

			change_pname("B");

			pid_t pid_D;
			int status_D;
			
			pid_D = fork();
			if (pid_D < 0) {
				perror("main: fork");
				exit(1);
			}
			if (pid_D == 0) {
				//i am D

				fork_procs("D",13);
				exit(1);
			}
			
			printf("D is created...\n");
			//B wait for D
			pid_D = wait(&status_D);
			explain_wait_status(pid_D, status_D);
		
			printf("B: Exiting...\n");	
			exit(19);
		}
		
		printf("B is created...\n");

		pid_C = fork();
		if (pid_C < 0) {
			perror("main: fork");
			exit(1);
		}
		if (pid_C == 0) {
			//i am C

			fork_procs("C",17);
		}
		printf("C is created...\n");

		//A waits for B
		pid_B = waitpid(pid_B, &status_B, 0);
		explain_wait_status(pid_B, status_B);
		
		//A wait for C
		pid_C = waitpid(pid_C, &status_C, 0);
		explain_wait_status(pid_C, status_C);

		printf("A: Exiting...\n");
		exit(16);
	}
	
	printf("A is created...\n");

	printf("Main is sleeping...\n");	
	sleep(SLEEP_TREE_SEC);

	show_pstree(getpid());

	printf("Main is waiting for A\n");
	pid_A = wait(&status_A);
	explain_wait_status(pid_A, status_A);

	return 0;
}