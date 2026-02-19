/*  Sample thread-safe implementation of a stack using a dynamically allocated
 *  linked list with forced deadlock */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define NQUEUE      1000    // Number of queues
#define MAX_DEPTH   10      // Max number of entries in one queue

struct _queue
{
    /*  Declare a mutex and a condvar.  We use the static initialiser so there is
     *  not need for us to call pthread_mutex_init */
    pthread_mutex_t lock; //  = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t condvar; //  = PTHREAD_COND_INITIALIZER;
    /* Declare full and empty flags */
    bool queueFull;
    bool queueEmpty;
    /* Declare a counter to keep track of how many nodes are in the list.  There is
     * no way to tell how many nodes are in the list other than walking the list and
     * counting them so maintaining this counter is more efficient */
    int nodeCount;
    /*  Declare a global pointer to the node structure to track the head of the list */
    struct node *firstNode;
    int index;
};

struct _queue queueArray[NQUEUE];

/*  Declare a struct that contains a name and a next pointer.  Note that the
 *  next pointer points to the structure type itself.  It is a self-referential
 *  struct */
struct node
{
        char *name;
        struct node *next;
};

/*  Declare a static (only visible in this file) function to push a new value to
 *  our linked list.  The new value always goes to the head of the list and so
 *  this makes the list behave like a stack (LIFO) */
static void __push (struct _queue *queue, char *name)
{
        struct node *node;
        /*  Allocate a new node to add to the list */
        node = malloc (sizeof (struct node));
        /*  Duplicate the name we have been passed */
        node->name = strdup (name);
	/*  Add the new node to the head of the list by making the next pointer
	 *  of this node point to whatever was the previous first node (which may have
	 *  been NULL if the list was empty) and then by making the firstNode point to
	 *  the new node so it is now the head of the list */
        node->next = queue->firstNode;
        queue->firstNode = node;
        queue->nodeCount++;
        
        queue->queueFull = (queue->nodeCount == MAX_DEPTH);
        queue->queueEmpty = false;

}

/*  This is the public method to push a value on to the stack.  It will block if
 *  the stack is full.  If it blocks, it will wait on the condvar to check again
 *  for space */
void push (struct _queue *queue, char *name)
{
        pthread_mutex_lock (&queue->lock);

        while (queue->queueFull)
            pthread_cond_wait (&queue->condvar, &queue->lock);

        __push (queue, name);

        /* Broadcast to all waiting threads that they should reevaluate
         * queueFull or queueEmpty flags */
        pthread_cond_broadcast (&queue->condvar);
        pthread_mutex_unlock (&queue->lock);
}

/*  Static function to pop the first element from our list.  If the list is
 *  empty then firstNode will be NULL and so we also return NULL.  If it is not
 *  empty, then change the firstnode to be the second node by changing it to be
 *  the value of the next pointer of the first node.  The node we just detached
 *  is the node to be popped.  We return the name in this node.  NOTE: we should
 *  free the memory used by the node and by the name parameter but for this demo
 *  we rely on the OS to clean up when we terminate */
static char * __pop (struct _queue *queue)
{
        struct node *node = queue->firstNode;

        if (node == NULL)
        {
                return NULL;
        }

        queue->firstNode = node->next;
        queue->nodeCount--;

        queue->queueEmpty = (queue->firstNode == NULL);

        return node->name;
}

static char * pop (struct _queue *queue)
{
        pthread_mutex_lock (&queue->lock);

        while (queue->queueEmpty)
            pthread_cond_wait (&queue->condvar, &queue->lock);

        char *ret = __pop (queue);
        pthread_cond_broadcast (&queue->condvar);
        pthread_mutex_unlock (&queue->lock);
        return ret;
}

/*  Function to move an item from one queue to another (pop followed by push) */

static void __move (struct _queue *from, struct _queue *to)
{
        __push (to, __pop (from));
}

/*  "public" wrapper function that locks both queues before calling the
 *  internal __move method.  THis is designed to deadlock due to random order of
 *  from and to locking.  Sooner or later a thread will try to form a circular
 *  lock and will deadlock */
static void move (struct _queue *from, struct _queue *to)
{
        pthread_mutex_lock (&from->lock);
        pthread_mutex_lock (&to->lock);

        while (from->queueEmpty)
            pthread_cond_wait (&from->condvar, &from->lock);

        __move (from, to);
        pthread_cond_broadcast (&from->condvar);
        pthread_mutex_unlock (&to->lock);
        pthread_mutex_unlock (&from->lock);
}

/*  Simple thread that sleeps for one second and then pushes a string to our
 *  stack.  Used to demonstrate that the main thread will wait for a non empty
 *  stack before returning from pop() */
void *thread_runner (void*arg)
{
        printf ("thread running\n");
        while (1)
        {
                struct _queue *from = &queueArray[rand () % 1000];
                struct _queue *to = &queueArray[rand () % 1000];
                printf ("[%d->%d]\n", from->index, to->index);
                move (from, to);
        }
}

/*  Declare a main function that accepts parameters.  The parameters provided
 *  are pushed on to each of our linked list one by one and then popped and printed.
 *  This should result in an output of the parameters in reverse order */
int main (int argc, char *argv[])
{
        for (int q = 0; q < 1000; q++)
        {
                pthread_mutex_init (&queueArray[q].lock, NULL);
                pthread_cond_init (&queueArray[q].condvar, NULL);
                queueArray[q].queueEmpty = true;
                queueArray[q].queueFull = false;
                queueArray[q].index = q;
                queueArray[q].firstNode = NULL;

                for (int i = 1; i < argc; i++)
                {
                        push(&queueArray[q], argv[i]);
                }
        }

        /*  Now create 1,000 threads */
        for (int q = 0; q < 1000; q++)
        {
                pthread_t id;
                pthread_create (&id, NULL, thread_runner, NULL);
        }

        /*  Sleep for one hour and let all the threads run.  We *could*
         *  implement some kind of counting scheme here that periodically checks
         *  how many items have moved in the last interval and then use this to
         *  detect deadlocks */
        sleep (3600);

        return 0;
}

