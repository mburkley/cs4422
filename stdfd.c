/*  Short program to demonstrate stdio file descriptors.  This program reads 5
 *  characters from fd 0 (stdin) and prints a message to stdout */

#include <unistd.h>
#include <stdio.h>

int main()
{
    char buffer[10];
    int count;

    count = read(0, buffer, 10);

    if ( count < 5)
    {
        fprintf (stderr, "Not enough characters\n");
        return -1;
    }

    buffer[count] = '\0';
    printf ("count=%d read=%s\n", count, buffer);
    return 0;
}

