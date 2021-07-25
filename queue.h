#ifndef QUEUE_H
#define QUEUE_H

struct queue_list
{
	struct queue_list * next;
	struct queue_list * prev;
	void * data;
};

struct queue
{
	struct queue_list * head;
	struct queue_list * tail;
};

size_t queue_size(struct queue const *);
void queue_add(struct queue *, void *);
void free_queue(struct queue *);
void free_queue_and_data(struct queue *);

#endif
