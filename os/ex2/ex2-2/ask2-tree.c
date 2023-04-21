#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "proc-common.h"
#include "tree.h"

void forker(struct tree_node *node) {
	if ((node->nr_children) == 0) {
		change_pname(node->name);
		sleep(10);
		printf("I, %s, die\n",node->name);
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
		for(int i=0; i<(node->nr_children); i++) {	
			waitpid(pid_arr[i],&status_arr[i],0);
			explain_wait_status(pid_arr[i],status_arr[i]);
		}
		printf("I, %s, die\n",node->name);
		exit(0);
	}
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
	
	sleep(3);
	show_pstree(pid);
	print_tree(root);
	
	pid=wait(&status);
	explain_wait_status(pid,status);
	
	return 0;
}