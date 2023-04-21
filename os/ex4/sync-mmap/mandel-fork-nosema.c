/*
 * mandel.c
 *
 * A program to draw the Mandelbrot Set on a 256-color xterm.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

/*TODO header file for m(un)map*/

#include "mandel-lib.h"

#define MANDEL_MAX_ITERATION 100000

/***************************
 * Compile-time parameters *
 ***************************/

/*
 * Output at the terminal is is x_chars wide by y_chars long
*/
int y_chars = 50;
int x_chars = 90;

/*
 * The part of the complex plane to be drawn:
 * upper left corner is (xmin, ymax), lower right corner is (xmax, ymin)
*/
double xmin = -1.8, xmax = 1.0;
double ymin = -1.0, ymax = 1.0;
	
/*
 * Every character in the final output is
 * xstep x ystep units wide on the complex plane.
 */
double xstep;
double ystep;

/*
 * This function computes a line of output
 * as an array of x_char color values.
 */
void compute_mandel_line(int line, int color_val[])
{
	/*
	 * x and y traverse the complex plane.
	 */
	double x, y;

	int n;
	int val;

	/* Find out the y value corresponding to this line */
	y = ymax - ystep * line;

	/* and iterate for all points on this line */
	for (x = xmin, n = 0; n < x_chars; x+= xstep, n++) {

		/* Compute the point's color value */
		val = mandel_iterations_at_point(x, y, MANDEL_MAX_ITERATION);
		if (val > 255)
			val = 255;

		/* And store it in the color_val[] array */
		val = xterm_color(val);
		color_val[n] = val;
	}
}

/*
 * This function outputs an array of x_char color values
 * to a 256-color xterm.
 */
void output_mandel_line(int fd, int color_val[])
{
	int i;
	
	char point ='@';
	char newline='\n';

	for (i = 0; i < x_chars; i++) {
		/* Set the current color, then output the point */
		set_xterm_color(fd, color_val[i]);
		if (write(fd, &point, 1) != 1) {
			perror("compute_and_output_mandel_line: write point");
			exit(1);
		}
	}

	/* Now that the line is done, output a newline character */
	if (write(fd, &newline, 1) != 1) {
		perror("compute_and_output_mandel_line: write newline");
		exit(1);
	}
}

void compute_and_output_mandel_line(int fd, int line)
{
	/*
	 * A temporary array, used to hold color values for the line being drawn
	 */
	int color_val[x_chars];

	compute_mandel_line(line, color_val);
	output_mandel_line(fd, color_val);
}

/*
 * Create a shared memory area, usable by all descendants of the calling
 * process.
 */
void *create_shared_memory_area(unsigned int numbytes)
{
	int pages;
	void *addr;

	if (numbytes == 0) {
		fprintf(stderr, "%s: internal error: called for numbytes == 0\n", __func__);
		exit(1);
	}

	/*
	 * Determine the number of pages needed, round up the requested number of
	 * pages
	 */
	pages = (numbytes - 1) / sysconf(_SC_PAGE_SIZE) + 1;

	/* Create a shared, anonymous mapping for this number of pages */
	/* TODO:  
		addr = mmap(...)
	*/
	addr = mmap(NULL, pages * sysconf(_SC_PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	return addr;
}

void destroy_shared_memory_area(void *addr, unsigned int numbytes) {
	int pages;

	if (numbytes == 0) {
		fprintf(stderr, "%s: internal error: called for numbytes == 0\n", __func__);
		exit(1);
	}

	/*
	 * Determine the number of pages needed, round up the requested number of
	 * pages
	 */
	pages = (numbytes - 1) / sysconf(_SC_PAGE_SIZE) + 1;

	if (munmap(addr, pages * sysconf(_SC_PAGE_SIZE)) == -1) {
		perror("destroy_shared_memory_area: munmap failed");
		exit(1);
	}
}

int safe_atoi(char *s, int *val)
{
	long l;
	char *endp;

	l = strtol(s, &endp, 10);
	if (s != endp && *endp == '\0') {
		*val = l;
		return 0;
	} else
		return -1;
}

void child_func(int line, int nprocs, int *buffer) {

	int i=0;

	for (i = line; i < y_chars; i += nprocs) {
		compute_mandel_line(i, &buffer[i * x_chars]);
	}
	exit(0);
}

int main(int argc, char **argv)
{
	int i;
	int nprocs;

	if (argc < 2) {
		fprintf(stderr, "You must provide at least two arguments!\n");
		exit(1);
	}

	if (safe_atoi(argv[1], &nprocs) < 0 || nprocs <= 0) {
		fprintf(stderr, "`%s' is not valid for `process count'\n", argv[1]);
		exit(1);
	}
	
	int *buffer;
	buffer = (int *)create_shared_memory_area((x_chars * y_chars) * sizeof(*buffer));
	
	xstep = (xmax - xmin) / x_chars;
	ystep = (ymax - ymin) / y_chars;
	
	/*
	 * draw the Mandelbrot Set, one line at a time.
	 * Output is sent to file descriptor '1', i.e., standard output.
	 */
	
	pid_t pid[nprocs];

	for (i = 0; i<nprocs; i++) {
		//compute_and_output_mandel_line(1, line);

		pid[i] = fork(); //fork tree root
		if (pid[i] < 0) {
			perror("main: fork");
			exit(1);
		}
		if (pid[i]==0) {
			child_func(i, nprocs, buffer);
			exit(1); //we should never reach this
		}
	}
	int status;
	for (i = 0; i<nprocs; i++) {
		if (-1 == waitpid(pid[i], &status, WUNTRACED)) {
			perror("waitpid");
		}
	}
	
	for (i = 0; i < y_chars; i++) {
		output_mandel_line(1, &buffer[i * x_chars]);
	}

	destroy_shared_memory_area(buffer, (x_chars * y_chars) * sizeof(*buffer));

	reset_xterm_color(1);
	return 0;
}