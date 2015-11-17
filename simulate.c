/*
 * simulate.c
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "simulate.h"
#include "mpi.h"


/* Add any global variables you may need. */


/* Add any functions you may need (like a worker) here. */


/*
 * Executes the entire simulation.
 *
 * i_max: how many data points are on a single wave
 * t_max: how many iterations the simulation should run
 * old_array: array of size i_max filled with data for t-1
 * current_array: array of size i_max filled with data for t
 * next_array: array of size i_max. You should fill this with t+1
 */
double *simulate(const int i_max, const int t_max, double *old_array,
		double *current_array, double *next_array)
{
	int rc, rank, num_tasks;

	rc = MPI_Init(NULL, NULL);

	/* Abort if MPI_Init did not succeed */
	if (rc != MPI_SUCCESS) {
		fprintf(stderr, "Unable to set up MPI");
		MPI_Abort(MPI_COMM_WORLD, rc);
	}

	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

	printf("%d of %d", rank, num_tasks);

	/* You should return a pointer to the array with the final results. */
	return current_array;
}
