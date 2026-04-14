#ifndef QUEUE_H
#define QUEUE_H 1
#include "dt.h"
void queue_init (TaskQueue *q);
void queue_push (TaskQueue *q, const TaskItem *item);
bool queue_pop (TaskQueue *q, TaskItem *out_item);
void queue_shutdown (TaskQueue *q);
void queue_destroy (TaskQueue *q);
#endif
