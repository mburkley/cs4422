#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

void *loop (void* param)
{
    while (1)
        ;

    return NULL;
}

int main (void)
{
    pthread_t tid;
    cpu_set_t cpuset;

    CPU_ZERO (&cpuset);
    CPU_SET (0, &cpuset);
    CPU_SET (1, &cpuset);

    for (int i = 0; i < 28; i++)
    {
        pthread_create (&tid, NULL, loop, NULL);
        pthread_setaffinity_np (tid, sizeof (cpuset), &cpuset);
    }

    sleep (120);
    return 0;
}

