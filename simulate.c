/*
 * Name: Doeke Leeuwis
 * Studentnr: 10723692
 * Studie: Informatica
 * 
 * simulate.c
 *
 * Wave equation simulation with IMP
 */

#include <stdio.h>
#include <stdlib.h>

#include "simulate.h"
#include "mpi.h"

/* Wave equation */
double wave_equation(int i, double *old, double *current) {
	double w;

	w = (2 * current[i]) - old[i];
	w += 0.15 * (current[i-1] - ((2 * current[i]) - current[i+1]));

	return w;
}

/*
 * Executes the entire simulation.
 *
 * i_max: how many data points are on a single wave
 * local_size: how many iterations the simulation should run
 * old_array: array of size i_max filled with data for t-1
 * current_array: array of size i_max filled with data for t
 * next_array: array of size i_max. You should fill this with t+1
 */
double *simulate(const int local_size, const int t_max, double *old_array,
		double *current_array, double *next_array)
{
	double *tmp_array;
	int left_neighbour, right_neighbour;
	int rank, num_tasks, t, i;
	MPI_Request request;
	MPI_Status status;

	/* Get rank of current process and total number of processes */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

    /* Set neighbors */
	left_neighbour = rank - 1;
	right_neighbour = rank + 1;

	/* Simulate t_max time steps */
	for (t = 0; t < t_max; t++) {

		/* Send to left neighbour (without blocking) */
		if (rank > 0)
			MPI_Isend(&current_array[1], 1, MPI_DOUBLE, left_neighbour, 3, MPI_COMM_WORLD, &request);

		/* Send to right neighbour (without blocking) */
		if (rank < num_tasks - 1)
			MPI_Isend(&current_array[local_size], 1, MPI_DOUBLE, right_neighbour, 4, MPI_COMM_WORLD, &request);

		/* Calculate local datapoints except halo cells */
		for (i = 2; i < local_size; i++) {
			next_array[i] = wave_equation(i, old_array, current_array);
		}

		/* Receive from left neighbour */
		if (rank > 0) {
			MPI_Recv(current_array, 1, MPI_DOUBLE, left_neighbour, 4, MPI_COMM_WORLD, &status);
			next_array[1] = wave_equation(1, old_array, current_array);
		}

		/* Receive from right neighbour */
		if (rank < num_tasks - 1) {
			MPI_Recv(&current_array[local_size+1], 1, MPI_DOUBLE, right_neighbour, 3, MPI_COMM_WORLD, &status);
			next_array[local_size] = wave_equation(local_size, old_array, current_array);
		}

		/* Swap the buffers around */
		tmp_array = old_array;
		old_array = current_array;
		current_array = next_array;
		next_array = tmp_array;
	}

	free(tmp_array);

	/* Returns a pointer to the array with the final results. */
	return current_array;
}
