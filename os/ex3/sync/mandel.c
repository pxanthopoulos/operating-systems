/*
 * mandel.c
 *
 * A program to draw the Mandelbrot Set on a 256-color xterm.
 *
 */

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>

#include "mandel-lib.h"

#define MANDEL_MAX_ITERATION 100000

#define perror_pthread(ret, msg) \
	do { errno = ret; perror(msg); } while (0)

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

/* Use Ctrl + C to stop the process
 * & reset the color of the terminal
 */
void Ctrlcproof(int sign) { 
    signal(sign, SIG_IGN);
    // Just ignore the sig
    reset_xterm_color(1);
	printf("\n");
    // move on w/ color reset
    exit(-1);
}

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

struct thread_info_struct {
	pthread_t tid; 	
	int thrid;
	int thrcnt;
	sem_t *sema_prev;
	sem_t *sema_next;
};

void *thread_start_fn(void *arg) {
	int i;
	struct thread_info_struct *thr = arg;

	for (i = thr->thrid; i < y_chars; i += thr->thrcnt) {
		int color_val[x_chars];

		compute_mandel_line(i, color_val);

		sem_wait(thr->sema_prev);
		output_mandel_line(1, color_val);
		sem_post(thr->sema_next);
	}
	return NULL;
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

void *safe_malloc(size_t size)
{
	void *p;

	if ((p = malloc(size)) == NULL) {
		fprintf(stderr, "Out of memory, failed to allocate %zd bytes\n",
			size);
		exit(1);
	}

	return p;
}

int main(int argc, char **argv) {

	signal(SIGINT, Ctrlcproof);
	
	int i;
	int nthreads;
	struct thread_info_struct *thr;

	if (safe_atoi(argv[1], &nthreads) < 0 || nthreads <= 0) {
		fprintf(stderr, "`%s' is not valid for `thread_count'\n", argv[1]);
		exit(1);
	}

	xstep = (xmax - xmin) / x_chars;
	ystep = (ymax - ymin) / y_chars;
	
	sem_t *sema;
	int ret;
	sema = safe_malloc(nthreads * sizeof(*sema));
	for(i = 0; i < nthreads; i++) {
		if (i == nthreads-1) {
			ret = sem_init(&sema[i], 0, 1);
			if (ret!=0) {
				fprintf(stderr, "Error in sem_init");
				exit(1);
			}
		}
		else { 
			ret = sem_init(&sema[i], 0, 0);
			if (ret!=0) {
				fprintf(stderr, "Error in sem_init");
				exit(1);
			}
		}
	}
	
	thr = safe_malloc(nthreads * sizeof(*thr));
	for (i = 0; i < nthreads; i++) {
		/* Initialize per-thread structure */
		thr[i].thrid = i;
		thr[i].thrcnt = nthreads;

		thr[i].sema_next = &sema[i];

		if (i == 0) {
			thr[i].sema_prev = &sema[nthreads-1];
		}
		else {
			thr[i].sema_prev = &sema[i-1];
		}
		
		/* Spawn new thread */
		ret = pthread_create(&thr[i].tid, NULL, thread_start_fn, &thr[i]);
		if (ret) {
			perror_pthread(ret, "pthread_create");
			exit(1);
		}
	}

	for (int i = 0; i < nthreads; i++) {
		sem_destroy(&sema[i]);
	}

	for (i = 0; i < nthreads; i++) {
		ret = pthread_join(thr[i].tid, NULL);
		if (ret) {
			perror_pthread(ret, "pthread_join");
			exit(1);
		}
	}

	reset_xterm_color(1);

	return 0;
}