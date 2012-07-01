
#include "animator.h"
#include <cmath>
#include <pthread.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

//keyboard & mouse numbers
#define ENTERKEY    13
#define ESCKEY      27
#define SCROLL_UP   3
#define SCROLL_DOWN 4

#define COLOR_BG    1.0, 1.0, 1.0
#define COLOR_FG    0.0, 0.0, 0.0
#define OPACITY     0.8

#define RADIUS      0.1
#define POLY_SIDES  10

namespace Animator
{

//glut callbacks
void display ();
void reshape (int w, int h);
void keyboard (unsigned char key, int w_x, int w_y) {
    switch (key) {
        case ESCKEY: pthread_exit(NULL);    break;
    }
}
//void special_keys (int key, int w_x, int w_y) {}
//void mouse (int button, int state, int X, int Y) {}
//void mouse_motion (int X, int Y) {}

//ctor and dtor
Window::Window (int argc, char **argv)
    : N(0), x(NULL), y(NULL), r(NULL)
{
    logger.info() << "initializing glut" |0;

    //define main window
    glutInit(&argc, argv);

    //set display mode
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA );

    //set window size
    glutInitWindowSize(600,600);
    glutInitWindowPosition(50,50);
    glutCreateWindow("Johann Animator");
    reshape(600,600);

    //set callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    //glutSpecialFunc(special_keys);
    //glutMouseFunc(mouse);
    //glutMotionFunc(mouse_motion);

    //set parameters
    glClearColor(COLOR_BG,0.0);
    glShadeModel(GL_FLAT);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
#ifndef CYGWIN_HACKS
    glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
#endif
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//-------- window identity --------

pthread_mutex_t Window::s_mutex = PTHREAD_MUTEX_INITIALIZER;
Window *window = NULL;
float radius;

void* main_loop (void*) {
    glutMainLoop();
    return NULL;
}
void Window::start ()
{
    //give control to glut
    window = this;
    logger.info() << "starting glut thread" |0;
    int status = pthread_create( &m_thread, NULL, main_loop, NULL);
    Assert (status == 0, "return code from pthread_create() is " << status);
}
void Window::update ()
{
    window = this;
    radius = RADIUS * sqrt(1.0 / (1 + N));
    glutPostRedisplay();
}

//-------- drawing --------

int width, height;
float x_scale, y_scale;
float convert_x (float x) { return x_scale * (2.0f * x / width - 1.0f); }
float convert_y (float y) { return y_scale * (1.0f - 2.0f * y / height); }

inline void draw_disc (float x, float y, float r)
{
    if (not std::isnormal(x)) return;
    if (not std::isnormal(y)) return;
    if (not std::isnormal(r)) return;

    glColor4f(COLOR_FG, OPACITY);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBegin(GL_POLYGON);
    float rad = radius * r;
    float ds = 2.0 * M_PI / POLY_SIDES;
    for (Int s=0; s<=POLY_SIDES; ++s) {
        glVertex2f(x + rad * cosf(s * ds),
                   y + rad * sinf(s * ds));
    }
    glEnd();
}
void display ()
{
    //clear buffer
    glClear(GL_COLOR_BUFFER_BIT);

    //draw obs
    window->lock();
    for (unsigned n=1; n<=window->N; ++n) {
        draw_disc (window->x[n],
                   window->y[n],
                   window->r[n]);
    }
    window->unlock();

    //DEBUG
    cout << '|' << std::flush;

    //swap buffers
    glutSwapBuffers();
}
void reshape (int w, int h)
{
    width = w;
    height = h;

    if (w <= h) { y_scale = 1.0f; x_scale = (1.0f * w)/ h; }
    else        { x_scale = 1.0f; y_scale = (1.0f * h)/ w; }

    //set viewport
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-x_scale, x_scale,  //x bounds
            -y_scale, y_scale,  //y bounds
            -8.0f, 8.0f);       //near and far clipping planes
}


}

