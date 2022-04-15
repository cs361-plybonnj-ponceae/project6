#include <stdlib.h>
#include <errno.h>
#include "intqueue.h"


/*
    This function initializes an empty queue. It takes the
    address of an existing struct intqueue, and sets the head
    and the tail pointers to NULL.
*/
void initqueue(struct intqueue *q) {
    q->head = NULL;
    q->tail = NULL;
}


/*
    This function takes the address of a struct intqueue and a value as
    its parameters. It then allocates memory for a new queue element,
    sets the value of the element to the one passed in, and inserts the new
    element at the end of the queue.
    Returns 0 on success and -1 when an error has occured.
*/
int enqueue(struct intqueue *q, int val) {
    struct iqelem *new; // pointer to new element
    new = (struct iqelem *)malloc(sizeof(struct iqelem)); // try to allocate space
    if (new == NULL)
        return -1;      // if malloc failed, return with error
    new->value = val;   // set the value of the new element
    new->prev = NULL;   // the new element has no previous element in the queue
    new->next = q->tail;    // the new element will become the new tail,
                            // so its next pointer needs to be the current tail
    if (q->tail != NULL) {  // if the queue is non-empty
        q->tail->prev = new;    // set the current last element's prev pointer
                                // to the new element
    }
    q->tail = new;      // the new element becomes the tail
    if (q->head == NULL) {  // if the queue had been empty
        q->head = q->tail;  // also set head to the new element
    }
    return 0;
}

/*
    This function takes the address of a struct intqueue, removes its first
    element from the queue if it exists, and returns that element's value.
    If the queue is empty, -1 is returned, and errno is set to EFAULT.
*/
int dequeue(struct intqueue *q) {
    int ret;    // return value; this is needed, because we free the memory
                // if the queue element before we return
    struct iqelem *to_remove;   // will point to the queue element we remove
    if (q->head == NULL) {      // if the queue is empty
        errno = EFAULT;         // set errno
        return -1;
    }
    to_remove = q->head;        // we will remove the first element in the queue
    ret = to_remove->value;     // save the return value
    if (q->head == q->tail) {   // if there is only a single element in the queue
        q->head = NULL;     // mark the queue as empty
        q->tail = NULL;
    }
    else {                  // if there is more than one element
        q->head = to_remove->prev; // the next element becomes the first
        if (q->head != NULL) {  // it really should not be NULL here, but make sure
            q->head->next = NULL; // the new head element does not have a next one
        }
    }
    free(to_remove);    // free the memory
    return ret;     // return the value
}

/*
    This function takes the address of a struct intqueue, and returns 1 is the
    queue is empty and 0 if there are elements in the queue.
*/
int isempty(struct intqueue *q) {
    return (q->head == NULL);   
}