#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

#include "proc-common.h"
#include "tree.h"

void computeval(int wfd, int rfd, char *op)
{
    int val1, val2;

    if (read(rfd, &val1, sizeof(val1)) != sizeof(val1)) 
    {
		perror("parent: read from pipe");
		exit(1);
	}

    if (read(rfd, &val2, sizeof(val2)) != sizeof(val2)) 
    {
		perror("parent: read from pipe");
		exit(1);
	}
	close(rfd);

    int res;

    if (strcmp(op, "+") == 0)
    {
        res = val1 + val2;
    }
    else
    {
        res = val1*val2;
    }

    if (write(wfd, &res, sizeof(res)) != sizeof(res)) 
    {
		perror("child: write to pipe");
        exit(1);
    }
	close(wfd);
}

void forker(struct tree_node *node, int fd) 
{
	if ((node->nr_children) == 0) 
	{   
        int val = atoi(node->name);
		if (write(fd, &val, sizeof(val)) != sizeof(val)) 
        {
		    perror("child: write to pipe");
            exit(1);
        }
		close(fd);
		exit(0);
	}
	else
	{   
        int pfd[2];
        //printf("Parent: Creating right pipe ...\n");
	    if (pipe(pfd) < 0) 
        {
	       	perror("pipe");
		    exit(1);
	    }

		pid_t pid_arr[node->nr_children];
		int status_arr[node->nr_children];

		for(int i = 0; i < node->nr_children; i++)
		{
			pid_arr[i] = fork();
			if (pid_arr[i] < 0)
			{
				perror("fork");
				exit(1);
			}
			if (pid_arr[i] == 0)
			{
				//i am child
				forker((node->children)+i, pfd[1]);
            	exit(1); //we should never reach this
			}
		}
		
        computeval(fd, pfd[0], node->name);

        for(int i=0; i<(node->nr_children); i++)
		{	
			//printf("I, %s, am waiting for %d child\n", node->name, i+1);
			waitpid(pid_arr[i],&status_arr[i],0);
			explain_wait_status(pid_arr[i],status_arr[i]);
		}
		exit(0);
    }
}

int main(int argc, char *argv[])
{
	pid_t p;
	int pfd[2];
	int status;
	int result;
    struct tree_node *root = get_tree_from_file(argv[1]);
	
	if (pipe(pfd) < 0) 
    {
		perror("pipe");
		exit(1);
	}

	p = fork();
	if (p < 0) 
    {
		/* fork failed */
		perror("fork");
		exit(1);
	}
	if (p == 0) 
    {
		/* In child process */
		forker(root, pfd[1]);
	}
    
	if (read(pfd[0], &result, sizeof(result)) != sizeof(result)) 
    {
		perror("parent: write to pipe (main)");
		exit(1);
	}
    close(pfd[0]);

	p = wait(&status);
	explain_wait_status(p, status);
	
    printf("Result is %d\n", result);

	return 0;
}