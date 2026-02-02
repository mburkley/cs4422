#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

/*  Include openGL and openGL utility toolkit (GLUT) header files */
#include <GL/glut.h> 
#include <GL/gl.h> 

/*  Hardcode the size of the display window in pixels.  Coordinates are from -1
 *  to +1 on both X and Y axes with the origin at 0,0 */
#define XSIZE 800
#define YSIZE 800

/*  Hard code some parameters */
#define NUM_THREADS 64
#define RADIUS 0.06f
#define SPEED 0.005f

/*  Define a struct to hold a region.  A region has boundary coordinates
 *  x1,y1 and x2,y2 as well as colour (rgb) and other attributes */
struct _region
{
    double x1;
    double x2;
    double y1;
    double y2;
    bool bounce;        // Do balls bounce when encountering this region
    bool lock;          // Does a ball acquire a lock to enter this region
    double r;
    double g;
    double b;
    pthread_mutex_t mutex;
};

/*  Declare 3 regions.  The first is the outher boundary with bounce set to true
 *  to ensure all balls stay inside the viewable area.  The next 2 are non
 *  bounce regions where a ball must acquire a lock to enter.  If one of these
 *  regions is not fully enclosed by the other then deadlock will occur */
struct _region region[] =
{
    { -0.99f, 0.99f, -0.99f, 0.99f, true, false, 1.0f,1.0f,1.0f, PTHREAD_MUTEX_INITIALIZER },
    { -0.20f, 0.20f, -0.20f, 0.40f, false, true, 1.0f,1.0f,1.0f, PTHREAD_MUTEX_INITIALIZER },
    { -0.10f, 0.10f, -0.40f, 0.20f, false, true, 1.0f,1.0f,1.0f, PTHREAD_MUTEX_INITIALIZER }
};

/*  Declare a region in which balls may NOT initially appear.  The coordinates
 *  should cover the locked regions above to ensure a ball does not appear
 *  inside a locked region */
struct _region startRegion =
{ -0.80f, 0.80f, -0.80f, 0.80f, false, true, 1.0f,1.0f,1.0f, PTHREAD_MUTEX_INITIALIZER };

/*  Define a struct to contain the attributes of a bouncing ball.  A ball has a
 *  position (x,y) and a vector (dx,dy).  It also has a colour (rgb), a radius
 *  and a thread handle since each ball has its own thread */
struct _ball
{
    double x;
    double y;
    double dx;
    double dy;
    double radius;
    double r;
    double g;
    double b;
    pthread_t thread;
}
ballThread[NUM_THREADS];

#define NUM_REGIONS (sizeof region / sizeof (struct _region))
#define NUM_SEGMENTS 30

int refreshMsec = 30;

/*  Return a random double between -1 and +1 */
static double randomf (double range)
{
    range *= ((2.0f * rand()) / (1.0f * RAND_MAX) - 1.0f);
    return range;
}

/*  Return true if any part of a ball is inside a region.  The position and
 *  radius of the ball are supplied */
bool insideRegion (double x, double y, double radius, struct _region *r)
{
    if (x-radius >= r->x1 && x+radius < r->x2 && y-radius >= r->y1 && y+radius < r->y2)
        return true;

    return false;
}

/*  Main thread runner for a bouncing ball.  A pointer to the ball struct is
 *  passed as a parameter */
void *ballMain (void *v)
{
    struct _ball *b = (struct _ball *) v;

    while (1)
    {
        for (int i = 0; i < NUM_REGIONS; i++)
        {
            struct _region *r = &region[i];

            /*  Check each of the regions in turn.  If the bounce attribute is
             *  set for a region then check is this ball about to hit a wall or
             *  not */
            if (r->bounce)
            {
                /*  Is the ball currently inside the region but its dx vector
                 *  will place it outside the region then revers its dx vector
                 *  (bounce off a vertical wall) */
                if (insideRegion (b->x, b->y, b->radius, r) && 
                    !insideRegion (b->x + b->dx, b->y, b->radius, r))
                {
                    b->dx = -b->dx;
                }


                /*  Is the ball currently inside the region but its dy vector
                 *  will place it outside the region then revers its dy vector
                 *  (bounce off a horizontal wall) */
                if (insideRegion (b->x, b->y, b->radius, r) && 
                    !insideRegion (b->x, b->y+b->dy, b->radius, r))
                {
                    b->dy = -b->dy;
                }
            }

            /*  Check the lock attribute of this region.  If a ball is entering
             *  the region then acquire the lock and set the colour of the
             *  region to be the colour of the ball.  If a ball is leaving the
             *  region then unlock the regions mutex and set the region wall
             *  colour back to white */
            if (r->lock)
            {
                if (!insideRegion (b->x, b->y, -b->radius, r) && 
                    insideRegion (b->x+b->dx, b->y+b->dy, -b->radius, r))
                {
                    pthread_mutex_lock (&r->mutex);
                    r->r = b->r;
                    r->g = b->g;
                    r->b = b->b;
                }

                if (insideRegion (b->x, b->y, -b->radius, r) && 
                    !insideRegion (b->x+b->dx, b->y+b->dy, -b->radius, r))
                {
                    r->r = 1.0f;
                    r->g = 1.0f;
                    r->b = 1.0f;
                    pthread_mutex_unlock (&r->mutex);
                }
            }
        }

        /*  Apply the vector delate and sleep for 20 msec */
        b->x += b->dx;
        b->y += b->dy;

        usleep (20000);
    }
}

void init()
{
    for (int i = 0; i < NUM_THREADS; i++)
    {
        struct _ball *b = &ballThread[i];
        b->radius = RADIUS;

        /*  Choose a random starting point for each ball but ensure it does not
         *  start within the prohibited start region */
        do
        {
            b->x = randomf (0.9);
            b->y = randomf (0.9);
        }
        while (insideRegion (b->x, b->y, -b->radius, &startRegion));

        /*  Assign each ball a random speed and colour */
        b->dx = randomf (SPEED);
        b->dy = randomf (SPEED);

        b->r = randomf (0.4) + 0.6;
        b->g = randomf (0.4) + 0.6;
        b->b = randomf (0.4) + 0.6;
        pthread_create (&b->thread, NULL, ballMain, b);
    }
}

void timer(int value)
{
    glutPostRedisplay();      // Post re-paint request to activate display()
    glutTimerFunc(refreshMsec, timer, 0); // next Timer call milliseconds later
}

/*  Draw a solid circle by actually drawing many triangles (segments). */
void drawCircle (double x, double y, double r)
{
    double angle;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f (x, y);
    for (int i = 0; i <= NUM_SEGMENTS; i++)
    {
        angle = (2 * M_PI * i) / NUM_SEGMENTS;
        glVertex2f (x + cos(angle) * r, y + sin(angle) * r);
    }
    glEnd();
}

void display (void)
{
    glClear(GL_COLOR_BUFFER_BIT);   // Clear the color buffer with current clearing color
        
    for (int i = 0; i < NUM_REGIONS; i++)
    {
        struct _region *r = &region[i];
        glColor3f (r->r, r->g, r->b);
        glLineWidth(5);
        glBegin(GL_LINE_LOOP);
        glVertex2f (r->x1, r->y1);
        glVertex2f (r->x2, r->y1);
        glVertex2f (r->x2, r->y2);
        glVertex2f (r->x1, r->y2);
        glEnd();
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        struct _ball *b = &ballThread[i];
        glColor3f (b->r, b->g, b->b);
        drawCircle (b->x, b->y, b->radius);
    }

    glutSwapBuffers();
}

void main(int argc, char**argv) 
{ 
    glutInit(&argc, argv); 
    glutInitWindowPosition(100,100); 
    glutInitWindowSize(XSIZE,YSIZE); 
    glutInitDisplayMode (GLUT_DOUBLE);
    glutCreateWindow("Thread Demo"); 
    glutDisplayFunc (display);
    glutTimerFunc(0, timer, 0);
    init();
    glutMainLoop(); 
}
