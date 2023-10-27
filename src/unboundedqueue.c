#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>

#include <common_utils.h>
#include <unboundedqueue.h>
#include <mythreads.h>

static inline Node_t  *allocNode()                  { Malloc(sizeof(Node_t),1);  }
static inline Queue_t *allocQueue()                 { Malloc(sizeof(Queue_t),1); }
static inline void freeNode(Node_t *node)           { myFree((void*)node); }
static inline void LockQueue(Queue_t *q)            { MutexLock(&q->qlock);   }
static inline void UnlockQueue(Queue_t *q)          { MutexUnlock(&q->qlock); }
static inline void UnlockQueueAndWait(Queue_t *q)   { CondWait(&q->qcond, &q->qlock); }
static inline void UnlockQueueAndSignal(Queue_t *q) { CondSignal(&q->qcond); MutexUnlock(&q->qlock); }


Queue_t *initQueue() {
    Queue_t *q = allocQueue();
    if (!q) return NULL;
    q->head = allocNode();
    if (!q->head) return NULL;
    q->head->data = NULL; 
    q->head->next = NULL;
    q->tail = q->head;    
    q->qlen = 0;
    MutexInit(&q->qlock);
    CondInit(&q->qcond);   
    return q;
}

void deleteQueue(Queue_t *q) {
    while(q->head != q->tail) {
	Node_t *p = (Node_t*)q->head;
	q->head = q->head->next;
	freeNode(p);
    }
    if (q->head) freeNode((void*)q->head);
    if (&q->qlock)  MutexDestroy(&q->qlock);
    if (&q->qcond)  CondDestroy(&q->qcond);
    myFree(q);
}

int push(Queue_t *q, void *data) {
    if ((q == NULL) || (data == NULL)) { errno= EINVAL; return -1;}
    Node_t *n = allocNode();
    if (!n) return -1;
    n->data = data; 
    n->next = NULL;

    LockQueue(q);
    q->tail->next = n;
    q->tail       = n;
    q->qlen      += 1;
    UnlockQueueAndSignal(q);
    return 0;
}

void *pop(Queue_t *q) {        
    if (q == NULL) { errno= EINVAL; return NULL;}
    LockQueue(q);
    while(q->head == q->tail) {
	UnlockQueueAndWait(q);
    }
    // locked
    assert(q->head->next);
    Node_t *n  = (Node_t *)q->head;
    void *data = (q->head->next)->data;
    q->head    = q->head->next;
    q->qlen   -= 1;
    assert(q->qlen>=0);
    UnlockQueue(q);
    freeNode(n);
    return data;
} 

unsigned long length(Queue_t *q) {
    LockQueue(q);
    unsigned long len = q->qlen;
    UnlockQueue(q);
    return len;
}
