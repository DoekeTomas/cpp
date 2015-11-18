/*
 * Name: Doeke Leeuwis
 * Studentnr: 10723692
 * Studie: Informatica
 *
 * assign3_1.c
 *
 * Contains code for setting up and finishing the simulation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "file.h"
#include "timer.h"
#include "simulate.h"
#include "mpi.h"

typedef double (*func_t)(double x);

/*
 * Simple gauss with mu=0, sigma^1=1
 */
double gauss(double x)
{
    return exp((-1 * x * x) / 2);
}


/*
 * Fills a given array with samples of a given function. This is used to fill
 * the initial arrays with some starting data, to run the simulation on.
 *
 * The first sample is placed at array index `offset'. `range' samples are
 * taken, so your array should be able to store at least offset+range doubles.
 * The function `f' is sampled `range' times between `sample_start' and
 * `sample_end'.
 */
void fill(double *array, int offset, int range, double sample_start,
        double sample_end, func_t f)
{
    int i;
    float dx;

    dx = (sample_end - sample_start) / range;
    for (i = 0; i < range; i++) {
        array[i + offset] = f(sample_start + i * dx);
    }
}


int main(int argc, char *argv[])
{
    double *old, *current, *next, *ret;
    int t_max, i_max;
    double time;

    int rc, rank, num_tasks, n;
    int local_size, local_start;
    MPI_Request request;
    MPI_Status status;

    /* Parse commandline args */
    if (argc < 3) {
        printf("Usage: %s i_max t_max num_threads [initial_data]\n", argv[0]);
        printf(" - i_max: number of discrete amplitude points, should be >2\n");
        printf(" - t_max: number of discrete timesteps, should be >=1\n");
        printf(" - num_threads: number of threads to use for simulation, "
                "should be >=1\n");
        printf(" - initial_data: select what data should be used for the first "
                "two generation.\n");
        printf("   Available options are:\n");
        printf("    * sin: one period of the sinus function at the start.\n");
        printf("    * sinfull: entire data is filled with the sinus.\n");
        printf("    * gauss: a single gauss-function at the start.\n");
        printf("    * file <2 filenames>: allows you to specify a file with on "
                "each line a float for both generations.\n");

        return EXIT_FAILURE;
    }

    i_max = atoi(argv[1]);
    t_max = atoi(argv[2]);

    if (i_max < 3) {
        printf("argument error: i_max should be >2.\n");
        return EXIT_FAILURE;
    }
    if (t_max < 1) {
        printf("argument error: t_max should be >=1.\n");
        return EXIT_FAILURE;
    }

    /* Initialize process */
    rc = MPI_Init(NULL, NULL);

    /* Abort if MPI_Init did not succeed */
    if (rc != MPI_SUCCESS) {
        fprintf(stderr, "Unable to set up MPI");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    /* Get rank of current process and total number of processes */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

    /* Calculate local buffer size of 1 process */
    local_size = i_max / num_tasks;

    /* Check if master */
    if (!rank) {
        /* Allocate and initialize buffers. */
        old = malloc(i_max * sizeof(double));
        current = malloc(i_max * sizeof(double));
        next = malloc(i_max * sizeof(double));

        if (old == NULL || current == NULL || next == NULL) {
            fprintf(stderr, "Could not allocate enough memory, aborting.\n");
            return EXIT_FAILURE;
        }

        memset(old, 0, i_max * sizeof(double));
        memset(current, 0, i_max * sizeof(double));
        memset(next, 0, i_max * sizeof(double));

        /* How should we will our first two generations? This is determined by the
         * optional further commandline arguments.
         * */
        if (argc > 3) {
            if (strcmp(argv[3], "sin") == 0) {
                fill(old, 1, i_max/4, 0, 2*3.14, sin);
                fill(current, 2, i_max/4, 0, 2*3.14, sin);
            } else if (strcmp(argv[3], "sinfull") == 0) {
                fill(old, 1, i_max-2, 0, 10*3.14, sin);
                fill(current, 2, i_max-3, 0, 10*3.14, sin);
            } else if (strcmp(argv[3], "gauss") == 0) {
                fill(old, 1, i_max/4, -3, 3, gauss);
                fill(current, 2, i_max/4, -3, 3, gauss);
            } else if (strcmp(argv[3], "file") == 0) {
                if (argc < 6) {
                    printf("No files specified!\n");
                    return EXIT_FAILURE;
                }
                file_read_double_array(argv[4], old, i_max);
                file_read_double_array(argv[5], current, i_max);
            } else {
                printf("Unknown initial mode: %s.\n", argv[3]);
                return EXIT_FAILURE;
            }
        } else {
            /* Default to sinus. */
            fill(old, 1, i_max/4, 0, 2*3.14, sin);
            fill(current, 2, i_max/4, 0, 2*3.14, sin);
        }

        timer_start();

        /* Send local buffers to workers (without blocking) */
        for (n = 1; n < num_tasks; n++) {
            local_start = n * local_size;
            MPI_Isend(&old[local_start], local_size, MPI_DOUBLE, n, 0, MPI_COMM_WORLD, &request);
            MPI_Isend(&current[local_start], local_size, MPI_DOUBLE, n, 1, MPI_COMM_WORLD, &request);
            MPI_Isend(&next[local_start], local_size, MPI_DOUBLE, n, 2, MPI_COMM_WORLD, &request);
        }

        /* Call the actual simulation in simulate.c. */
        current = simulate(local_size, t_max, old, current, next);

        /* Receive results from workers */
        for (n = 1; n < num_tasks; n++) {
            local_start = n * local_size;
            MPI_Recv(&current[local_start], local_size, MPI_DOUBLE, n, 1, MPI_COMM_WORLD, &status);
        }

        time = timer_end();

        printf("Took %g seconds\n", time);
        printf("Normalized: %g seconds\n", time / (1. * i_max * t_max));

        file_write_double_array("result.txt", current, i_max);
    }

    /* Run if worker */
    else {
        /* Allocate and initialize buffers. */
        old = malloc((local_size + 2) * sizeof(double));
        current = malloc((local_size + 2) * sizeof(double));
        next = malloc((local_size + 2) * sizeof(double));

        /* Receive local buffer from master */
        MPI_Recv(&old[1], local_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&current[1], local_size, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&next[1], local_size, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD, &status);

        /* Call the actual simulation in simulate.c. */
        ret = simulate(local_size, t_max, old, current, next);

        /* Sends results to master */
        MPI_Send(&current[1], local_size, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
    }

    /* Can't free old and current for some reason..
     * Error: malloc: *** error for object <pointer>: pointer being freed was not allocated
     */
    //free(old);
    //free(current);
    free(next);

    MPI_Finalize();
    return EXIT_SUCCESS;
}
