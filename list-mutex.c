/*  C code that implements a simple doubly-linked list.  A global head and tail
 *  pointer are maintained and each node in the list has a next and prev
 *  pointer.  A single mutex protects access to the head and tail pointers */

#include <stdio.h>      // printf and friends
#include <stdlib.h>     // malloc, exit and others
#include <string.h>     // strcmp, strdup, ...
#include <pthread.h>    // mutex and cond var functions

struct node;

struct node
{
    struct node *prev;
    struct node *next;
    char *name;
};

struct node *tail = NULL;
struct node *head = NULL;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*  Add a node to the tail of the list.  Memory for the node is dynamically
 *  allocated.  The contents of the string parameter are copied and allocated
 *  separately using strdup */
void addNode (char *name)
{
    struct node *node = malloc (sizeof (node));
    node->name = strdup (name);
    node->next = NULL;
    node->prev = NULL;

    pthread_mutex_lock (&mutex);

    if (tail == NULL)
    {
        head = tail = node;
    }
    else
    {
        tail->next = node;
        node->prev = tail;
        tail = node;
    }

    pthread_mutex_unlock (&mutex);
}

/*  Delete a named node from the list.  The node can appear anywhere in the
 *  list.  If no matching node is found, the function returns a non zero value.
 *  If a node is found and successfully deleted then zero is returned */

int delNode (char *name)
{
    pthread_mutex_lock (&mutex);

    struct node *n = head;

    for (n = head; n != NULL; n = n->next)
    {
        if (!strcmp (n->name, name))
        {
            if (n->next)
                n->next->prev = n->prev;

            if (n->prev)
                n->prev->next = n->next;

            if (n == head)
                head = n->next;

            if (n == tail)
                tail = n->prev;

            free (n->name);
            free (n);

            /*  This function returns here if an entry is found.  NOTE that the
             *  mutex unlock call has been commented out to simulate a forgotten
             *  unlock.  This will cause the next operation to block
             *  indefinitely as it will not be able to acquire the lock.  The
             *  key item to note is the bug (infinite block) will not manifest
             *  in the function with the error.  It will appear as if this
             *  funciton completed correctly. */
            // pthread_mutex_unlock (&mutex);
            return 0;
        }
    }

    /*  No matching node was found, return a non zero value to indicate error */
    pthread_mutex_unlock (&mutex);
    return -1;
}

/*  This fucntion just lists all nodes in the list.  Nodes are numbered using a
 *  counter that is pre-incremented so as to start from 1 */
void listNodes (void)
{
    struct node *n;
    int count = 0;

    for (n = head; n != NULL; n = n->next)
    {
        printf ("node %d : %s", ++count, n->name);
    }
}

int main(void)
{
    char s[100];

    /*  Loop forever */
    while (1)
    {
        printf ("(A)dd, (L)ist or (D)elete?\n");
        fgets (s, sizeof s, stdin);

        /*  Check the first character of the string that was read from stdin.
         *  We could also have used a switch statement.  We could also have used
         *  toupper() to prevent the need to check both upper and lower case.  */
        if (s[0] == 'A' || s[0] == 'a')
        {
            printf ("name? ");
            fgets (s, sizeof s, stdin);
            addNode (s);
        }
        else if (s[0] == 'D' || s[0] == 'd')
        {
            printf ("name? ");
            fgets (s, sizeof s, stdin);
            int result = delNode (s);
            printf ("Result = %s\n", result ? "failure" : "success");
        }
        else if (s[0] == 'L' || s[0] == 'l')
        {
            listNodes();
        }
        else
            printf ("hmmm?\n");
    }
    return 0;
}
