/* intqueue.h */

#ifndef __CS361_INTQUEUE__
#define __CS361_INTQUEUE__

/*
    This is the main data structure for the queue. It contains pointers
    to the first and last element in the queue.
*/
struct intqueue {
    struct iqelem *head;
    struct iqelem *tail;
};

/* 
    Queue elements are linked together in a doubly-linked list. There
    are next and previous pointers, and the integer value.
*/
struct iqelem {
    struct iqelem *next;
    struct iqelem *prev;
    int value;
};

void initqueue(struct intqueue *q);
int enqueue(struct intqueue *q, int val);
int dequeue(struct intqueue *q);
int isempty(struct intqueue *q);

#endif