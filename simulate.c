/*
 * Name: Doeke Leeuwis
 * Studentnr: 10723692
 * Studie: Informatica
 * 
 * simulate.c
 *
 * Wave equation simulation with PThreads
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "simulate.h"

/* Global variables */
double *old_array;
double *current_array;
double *next_array;

int data_points;
int threads;

/* Wave equation */
void *wave_equation(void *thread_id) {
	int i, i_start, i_end;

	/* Convert thread id to int */
	int *tid_p = (int *)thread_id;
	int tid = *tid_p;

	/* Set the start and end data points */
	i_start = ((double)tid / (double)threads) * data_points;
	i_end = ((double)(tid + 1) / (double)threads) * data_points;

	if (i_start == 0)
		i_start = 1;
	if (i_end == data_points)
		i_end = data_points - 1;

	/* Calculate data points i_start till i_end */
	for (i = i_start; i < i_end - 1; i++) {

		/* Wave equation */
		next_array[i] = (2 * current_array[i]) - old_array[i];
		next_array[i] += 0.15 * (current_array[i-1] - ((2 * current_array[i]) - current_array[i+1]));
	}

	return thread_id;
}

/*
 * Executes the entire simulation.
 *
 * i_max: how many data points are on a single wave
 * t_max: how many iterations the simulation should run
 * num_threads: how many threads to use (excluding the main threads)
 * pnt_old_array: array of size i_max filled with data for t-1
 * pnt_current_array: array of size i_max filled with data for t
 * pnt_next_array: array of size i_max. You should fill this with t+1
 */
double *simulate(const int i_max, const int t_max, const int num_threads,
	double *pnt_old_array, double *pnt_current_array, double *pnt_next_array) {

	int t, i, *id;
	double *tmp_array;
	pthread_t thread_ids[num_threads];
	void *result;

	old_array = pnt_old_array;
	current_array = pnt_current_array;
	next_array = pnt_next_array;

	data_points = i_max;
	threads = num_threads;

	/* Simulate t_max timesteps */
	for (t = 0; t < t_max; t++) {

		/* Create num_threads threads */
		for (i = 0; i < num_threads; i++) {

			/* Create pointer to id */
			id = (int *)malloc(sizeof(int));
			*id = i;

			pthread_create(&thread_ids[i], NULL, &wave_equation, id);
		}

		/* Wait till all threads are finished */
		for (i = 0; i < num_threads; i ++) {
			pthread_join(thread_ids[i], &result);
			free(result);
		}

		/* Swap the buffers around */
		tmp_array = old_array;
		old_array = current_array;
		current_array = next_array;
		next_array = tmp_array;
	}

	/* Return the final results. */
	return current_array;
}
