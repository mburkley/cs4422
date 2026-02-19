/*  Sample thread-safe implementation of a stack using a dynamically allocated
 *  linked list */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

/*  Declare a mutex and a condvar.  We use the static initialiser so there is
 *  not need for us to call pthread_mutex_init */
pthread_mutex_t marks_thread_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t marks_condvar = PTHREAD_COND_INITIALIZER;
/* Declare full and empty flags */
bool queueFull;
bool queueEmpty = true;
/* Declare a counter to keep track of how many nodes are in the list.  There is
 * no way to tell how many nodes are in the list other than walking the list and
 * counting them so maintaining this counter is more efficient */
int nodeCount;

/*  The maximum number of nodes we allow in the list (stack) */
#define MAX_DEPTH 10

/*  Declare a struct that contains a name and a next pointer.  Note that the
 *  next pointer points to the structure type itself.  It is a self-referential
 *  struct */
struct node
{
        char *name;
        struct node *next;
};

/*  Declare a global pointer to the node structure to track the head of the list */
struct node *firstNode = NULL;

/*  Declare a static (only visible in this file) function to push a new value to
 *  our linked list.  The new value always goes to the head of the list and so
 *  this makes the list behave like a stack (LIFO) */
static void __push (char *name)
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
        node->next = firstNode;
        firstNode = node;
        nodeCount++;
        
        queueFull = (nodeCount == MAX_DEPTH);
        queueEmpty = false;

}

/*  This is the public method to push a value on to the stack.  It will block if
 *  the stack is full.  If it blocks, it will wait on the condvar to check again
 *  for space */
void push (char *name)
{
        pthread_mutex_lock (&marks_thread_lock);

        while (queueFull)
            pthread_cond_wait (&marks_condvar, &marks_thread_lock);

        __push (name);

        /* Broadcast to all waiting threads that they should reevaluate
         * queueFull or queueEmpty flags */
        pthread_cond_broadcast (&marks_condvar);
        pthread_mutex_unlock (&marks_thread_lock);
}

/*  Static function to pop the first element from our list.  If the list is
 *  empty then firstNode will be NULL and so we also return NULL.  If it is not
 *  empty, then change the firstnode to be the second node by changing it to be
 *  the value of the next pointer of the first node.  The node we just detached
 *  is the node to be popped.  We return the name in this node.  NOTE: we should
 *  free the memory used by the node and by the name parameter but for this demo
 *  we rely on the OS to clean up when we terminate */
static char * __pop (void)
{
        struct node *node = firstNode;

        if (node == NULL)
        {
                return NULL;
        }

        firstNode = node->next;
        nodeCount--;

        queueEmpty = (firstNode == NULL);

        return node->name;
}

static char * pop (void)
{
        pthread_mutex_lock (&marks_thread_lock);

        while (queueEmpty)
            pthread_cond_wait (&marks_condvar, &marks_thread_lock);

        char *ret = __pop ();
        pthread_cond_broadcast (&marks_condvar);
        pthread_mutex_unlock (&marks_thread_lock);
        return ret;
}

/*  Simple thread that sleeps for one second and then pushes a string to our
 *  stack.  Used to demonstrate that the main thread will wait for a non empty
 *  stack before returning from pop() */
void *thread_runner (void*arg)
{
    printf ("thread running\n");
    sleep (1);
    printf ("push value\n");
    push ("end");
}

/*  Declare a main function that accepts parameters.  The parameters provided
 *  are pushed on to our linked list one by one and then popped and printed.
 *  This should result in an output of the parameters in reverse order */
int main (int argc, char *argv[])
{
        for (int i = 1; i < argc; i++)
        {
                push(argv[i]);
        }

        char *name;

        pthread_t id;
        pthread_create (&id, NULL, thread_runner, NULL);

        /*  Pop the parameters from the stack.  Note this function will not
         *  return as it will block whenever the stack is empty */
        do
        {
                name = pop();
                if (name)
                        printf ("name is %s (pointer is %p)\n", name, name);
        }
        while (name != NULL);

        return 0;
}

