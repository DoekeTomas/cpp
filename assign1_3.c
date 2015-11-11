/*
 * Name: Doeke Leeuwis
 * Studentnr: 10723692
 * Studie: Informatica
 *
 * assign1_3.c
 *
 * Makes a list of prime numbers
 *
 * usage:
 *   $ make
 *   $ ./assign1_3
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


/* Nodes will be used to create a linked list */
typedef struct node
{
	int nr;
	struct node *next;
} node;


int main(int argc, char *argv[]) {
	int i, n;
	node *start_node, *prev_node, *cur_node, *next_node;

	start_node = (node *)malloc(sizeof(node));
	start_node->nr = 2;
	start_node->next = NULL; 

	prev_node = start_node;

	/* Create list of natural numbers */
	for (i = 3; i < 100000; i++) {
		next_node = (node *)malloc(sizeof(node));
		next_node->nr = i;
		next_node->next = NULL;

		prev_node->next = next_node;
		prev_node = next_node;
	}

	/* Sieve of Eratosthenes algorithm */
	while (start_node) {
		printf("%d, ", start_node->nr);

		n = start_node->nr;
		cur_node = start_node;

		while (cur_node->next) {
			next_node = cur_node->next;

			if (!(next_node->nr % n)) {
				cur_node->next = next_node->next;
				free(next_node);
			}
			else {
				cur_node = next_node;
			}
		}

		start_node = start_node->next;
	}

	printf(" ...\n");
	return EXIT_SUCCESS;
}
