/*  Simple demonstration of fork() function call.  This program calls fork()
 *  once but the output line appears twice.  The return value from fork() is
 *  different for each return */
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    pid_t pid;  
    pid=fork();
    printf("pid=%d\n", (int)pid);
}

