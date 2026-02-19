#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main()
{
    pid_t pid;
    int fds[2];

    /*  Create a pipe.  Pass in an array of two ints to receive the two file
     *  desciptors (one for read, one for write) */
    pipe (fds);

    /* Fork the process.  This will create a child and a parent each with its
     * own copy of the two file descriptors */
    pid = fork ();

    if (pid == 0)
    {
        /*  If the return is zero then this is the parent.  Print the fds, close the
         *  read end and write a message to the write end of the pipe. */
        printf ("parent, fd0=%d fd1=%d\n", fds[0], fds[1]);
        close (fds[0]);
        
        /*  For convenience, lets duplicate the write end of the pipe to the
         *  stdout file number so now all output to stdout goes to the pipe
         *  instead */
        dup2 (fds[1], STDOUT_FILENO);
        char *msg = "hello from parent\n";
        // write (fds[1], msg, strlen (msg)); 
        printf ("%s", msg);
    }
    else
    {
        /*  If the pid is non-zero then this is the child end of the pipe.  We
         *  can close the write end and read from the read end of the pipe.  We
         *  declare an arbitrary buffer size that we know will be bigger than
         *  the message being sent by the parent */
        printf ("child, fd0=%d fd1=%d\n", fds[0], fds[1]);
        close (fds[1]);
        char buffer[100];
        int count;
        count = read (fds[0], buffer, 100);
        buffer[count] = '\0';
        printf ("child received '%s'\n", buffer);
    }
    return 0;
}
