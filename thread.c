#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>

/*  Declare a structure to track info specific to one thread.  In this case we
 *  just record the thread id and the thread index */
struct threadInfo
{
    pthread_t threadId;
    int index;
};

/*  Declare an array of 10 array info structs */
struct threadInfo threadArray[10];

/*  Global boolean flag to tell threads to keep running.

    NOTE: this is technically creating a race condition as all threads are
    accessing a non atomic common global value with no mutex protection.
    However we are going to use the knowledge that reading or writing a single
    int value is a thread safe operation */
int continueRunning = true;

/*  Declare a function to run a thread.  All threads run this same function.
 *  The input and return parameters are both void* to allow flexibility.  We
 *  cast our threadInfo struct to a void* to pass as a parameter.  Note the
 *  order of the running and stopping output messages. */
void *runner (void *arg)
{
    struct threadInfo *t = (struct threadInfo *) arg;
    printf ("thread %d running ...\n", t->index);

    /*  Test the global boolean and keep sleeping for 1 second at a time so long
     *  as it is true */
    while (continueRunning)
        sleep (1);

    printf ("thread %d stopping ...\n", t->index);
}

/*  A global signal handler that clears the continue running flag.

    NOTE: signals are not thread safe.  We don't know which of our threads will
    receive this signal.  In this case it doesn't matter and we don't care since
    there is only a single global flag so it doesn't matter who clears it, just
    that it has been cleared.  But in a non trivial system we would avoid using
    signals in multi-threaded apps */
void signalHandler (int sig)
{
    printf ("signal seen\n");
    continueRunning = 0;
}

int main()
{
    /*  Capture the ctrl-c signal to gracefully terminate our program */
    signal (SIGINT, signalHandler);

    /*  Create 10 thread instances */
    for (int i = 0; i < 10; i++)
    {
        threadArray[i].index = i;
        pthread_create (&threadArray[i].threadId, NULL, runner, &threadArray[i]);
    }

    /*  Wait for 10 thread instances to complete.  Note the order of the output
     */
    for (int i = 0; i < 10; i++)
    {
        pthread_join (threadArray[i].threadId, NULL);
        printf ("thread %d joined ...\n", i);
    }

    return 0;

}
