#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int main(void)
{
    int fd = open ("/tmp/foo", O_RDWR|O_CREAT);
    ftruncate (fd, 10240);

    void *p=mmap (NULL, 10240, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    printf("p=%p\n", p);
    int *x = (int*) p;
    x[0]=0x6080;
    return 0;

}

