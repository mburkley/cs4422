/*  Program that analyses the /proc/<pid>/maps file for itself or another pid
 *  provided on the command line.  It loops while allocating heap memory and
 *  reporting any changes in the mmap'd heap memory size */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

/*  Tracks the heap size and compares to a static value.  Only reports a change
 *  if the new size differs from previous. */
static void analyseHeap (int size)
{
    static int lastSize;

    if (size != lastSize)
    {
        printf ("heap size=%d (change=%x)\n", size, size-lastSize);
        lastSize = size;
    }
}

/*  Parses one line from a maps file.  It reads the values from each line of the
 *  file and if the allocation is for heap then it calls the analyse heap
 *  function */
static void parseLine (char *line)
{
    int from;
    int to;
    char perms[5];
    int offset;
    char dev[20];
    int inode;
    char path[200];

    /*  Anonymous entries do not have a path.  To avoid ... */
    strcpy (path, "[empty]");

    /*  We can't proceed unless we scan at least 6 items */
    if (sscanf (line, "%x-%x %s %x %s %d %s",
                &from, &to, perms, &offset, dev, &inode, path) < 6)
    {
        fprintf (stderr, "Parse error on %s", line);
        exit (EXIT_FAILURE);
    }
    if (!strcmp (path, "[heap]"))
    {
        analyseHeap (to-from);
    }
}

/*  main optionally accepts one numeric argument for a program pid */
int main (int argc, char *argv[])
{
    pid_t pid;
    FILE *file;
    char filename[100];
    char line[1000];

    /*  If an argument is supplied, get its integer value.  Otherwise call
     *  getpid() to find our own pid.  We could also use /proc/self/maps */
    if (argc > 1)
        pid = atoi (argv[1]);
    else
        pid = getpid ();

    sprintf (filename, "/proc/%d/maps", pid);
    file = fopen (filename, "r");

    if (file == NULL)
    {
        fprintf (stderr, "Can't find process id %d\n", pid);
        exit (EXIT_FAILURE);
    }

    char *p1;
    while (1)
    {
        /*  Since we are in a loop, we want to reread the file from the
         *  beginning so seek to offset 0 */
        fseek (file, 0, SEEK_SET);

        while (fgets (line, sizeof (line), file) != NULL)
        {
            parseLine (line);
        }

        /*  Allocate a small random amount of data and sleep for 200 usecs. */
        p1 = malloc (37);
        usleep (200);
    }

    return 0;
}
