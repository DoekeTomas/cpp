/*
 * Name: Doeke Leeuwis
 * Studentnr: 10723692
 * Studie: Informatica
 * 
 * simulate.c
 *
 * Wave equation simulation with omp
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "omp.h"
#include "simulate.h"

/*
 * Executes the entire simulation.
 *
 * i_max: how many data points are on a single wave
 * t_max: how many iterations the simulation should run
 * num_threads: how many threads to use
 * old_array: array of size i_max filled with data for t-1
 * current_array: array of size i_max filled with data for t
 * next_array: array of size i_max. You should fill this with t+1
 */
double *simulate(const int i_max, const int t_max, const int num_threads,
        double *old_array, double *current_array, double *next_array)
{
    int t, i;
	double *tmp_array;

	/* Set max number of threads */
	omp_set_num_threads(num_threads);

    /* Simulate t_max timesteps */
	for (t = 0; t < t_max; t++) {

		/* Parallelise the wave equation */
		#pragma omp parallel for
	    for (i = 1; i < i_max - 1; i++) {

			/* Wave equation */
			next_array[i] = (2 * current_array[i]) - old_array[i];
			next_array[i] += 0.15 * (current_array[i-1] - ((2 * current_array[i]) - current_array[i+1]));
		}

		/* Swap the buffers around */
		tmp_array = old_array;
		old_array = current_array;
		current_array = next_array;
		next_array = tmp_array;
	}

	return current_array;
}
