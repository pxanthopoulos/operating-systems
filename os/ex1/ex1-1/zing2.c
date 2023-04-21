#include <unistd.h>
#include <stdio.h>

void zing(void) { 
	char *name;
	name = getlogin();
	printf("This is: %s\n", name);

}