#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "tree.h"
#include "proc-common.h"

void forker(struct tree_node *node) {
	if ((node->nr_children) == 0) {
		change_pname(node->name);
		raise(SIGSTOP);
		printf("%s is awake\n", node->name);
		exit(0);
	}
	else {
		pid_t pid_arr[node->nr_children];
		int status_arr[node->nr_children];
		change_pname(node->name);

		for (int i=0; i<(node->nr_children); i++) {
			pid_arr[i] = fork();
			if (pid_arr[i] < 0) {
				perror("fork");
				exit(1);
			}
			if (pid_arr[i] == 0) {
				//i am child
				printf("%s is created\n", ((node->children)+i)->name);
				forker((node->children)+i);
				exit(1); //we should never reach this
			}
		}

		wait_for_ready_children(node->nr_children);

		raise(SIGSTOP);

		printf("%s is awake\n", node->name);
		
		for(int i=0; i<(node->nr_children); i++) {	
			kill(pid_arr[i], SIGCONT);
			waitpid(pid_arr[i],&status_arr[i],0);
			explain_wait_status(pid_arr[i],status_arr[i]);
		}
	}
	exit(0);
}

int main(int argc, char *argv[]) {
	pid_t pid;
	struct tree_node *root;
	int status;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <input_tree_file>\n\n", argv[0]);
		exit(1);
	}

	root = get_tree_from_file(argv[1]);

	pid = fork(); //fork tree root
	if (pid < 0) {
		perror("main: fork");
		exit(1);
	}
	if (pid==0) {
		//i am the root
		printf("%s is created\n", root->name);
		forker(root);
		exit(1); //we should never reach this
	}
	
	wait_for_ready_children(1);

	show_pstree(pid);
	print_tree(root);

	kill(pid, SIGCONT);

	wait(&status);
	explain_wait_status(pid, status);

	return 0;
}