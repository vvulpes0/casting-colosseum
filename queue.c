#include <stdlib.h>
#include <stdio.h>

#include "queue.h"

size_t
queue_size(struct queue const * this)
{
	if (!this) {return 0;}
	struct queue_list * x = this->head;
	size_t n = 0;
	while (x)
	{
		++n;
		x = x->next;
	}
	return n;
}

void
queue_add(struct queue * this, void * data)
{
	if (!this) {return;}
	struct queue_list * x = malloc(sizeof(struct queue_list));
	if (!x) {return;}
	x->next = NULL;
	x->data = data;
	x->prev = this->tail;
	if (this->tail) {this->tail->next = x;}
	this->tail = x;
	if (!this->head) {this->head = x;}
}

void
free_queue_and_data(struct queue * this)
{
	if (!this) {return;}
	if (!this->head) {return;}
	struct queue_list * x;
	while ((x = this->head))
	{
		this->head = this->head->next;
		free(x->data);
		free(x);
	}
	this->tail = NULL;
}

void
free_queue(struct queue * this)
{
	if (!this) {return;}
	if (!this->head) {return;}
	struct queue_list * x;
	while ((x = this->head))
	{
		this->head = this->head->next;
		free(x);
	}
	this->tail = NULL;
}
