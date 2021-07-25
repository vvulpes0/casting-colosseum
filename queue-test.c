#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

int
main(void)
{
	struct queue q = {.head = NULL, .tail = NULL};
	printf("size of empty: %lu\n", queue_size(&q));
	int * x = malloc(sizeof(int));
	if (x) {*x = 12; queue_add(&q, x);}
	printf("size of one: %lu\n", queue_size(&q));
	x = malloc(sizeof(int));
	if (x) {*x = 12; queue_add(&q, x);}
	printf("size of two: %lu\n", queue_size(&q));
	free_queue_and_data(&q);
	return 0;
}
