/*  Program that continuously allocates heap memory to demonstrate that the heap
 *  map is updated periodically to grow the heap.  Run this program in the
 *  backgroup and monitor its /proc/maps file */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main ()
{
    char *p1 = malloc (1);
    char *p2 = malloc (9);

    printf ("p1=%p p2=%p\n", p1, p2);
    printf ("diff=%ld\n", p2-p1);
    
    while (1)
    {
        usleep (100);
        p1=malloc(1000);
        p1[42]=7;
    }
}

