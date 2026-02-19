/*  Program that calls mmap to map a file into memory using mmap.  If a file
 *  named mapfile does not exist then an anonymous mapping is created. */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int main ()
{
    int fd = open ("mapfile", O_RDWR);
    printf("fd=%d\n", fd);
    char *p;

    if (fd == -1)
        p = mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, fd, 0);
    else
        p = mmap(NULL, 8192, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_SHARED, fd, 0);

    if (p == MAP_FAILED)
    {
        perror ("mmap");
        exit (1);
    }

    printf("p=%p\n", p);
    p[4097]=42;
    sleep(100000);

}
