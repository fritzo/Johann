
/** map viewer using OpenGL.
 *
 * TODO
 * (T1) add variable drag channels
 * (T2) switch from hand-multiplied matrices to OpenGL matrix projections.
 */

#include "graph.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

#if defined(__APPLE__) && defined(__MACH__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define LOG(mess) { std::cout << mess << std::endl; }
#define DEBUG(mess) LOG("DEBUG " << mess)
#define PRINT(arg) LOG(#arg " = " << arg)
#define ERROR(mess) { std::cout \
    << "\n\t" << mess << "\n\t" \
    << __FILE__ << " : " << __LINE__ << "\n\t" \
    << __PRETTY_FUNCTION__ << std::endl; \
    exit(0); }
#define TODO() { ERROR("control reached unfinished code:"); }
#define ASSERT(cond,mess) { if (!(cond)) ERROR(mess); }

//keyboard & mouse numbers
#define ENTERKEY    13
#define ESCKEY      27
#define SCROLL_UP   3
#define SCROLL_DOWN 4

#define COLOR_BG    0.0, 0.0, 0.0
#define COLOR_FG    1.0, 1.0, 1.0
#define VERT_OPACITY 0.2
#define EDGE_OPACITY 0.4

#define POLY_SIDES  10

#define INIT_RAD    0.5f

#define INIT_ZOOM   12.0f
#define ZOOM_IN     1.2f
#define ZOOM_OUT    1.0f/1.2f
#define ANGLE       2.0f

#define SIMPLEST    24

inline float color_map (float t) { return 0.2 + 0.8 * t; }
inline void reverse (float& t) { t = 1.0f - t; }

//----( convenience math )----------------------------------------------------

template<class T> inline T sqr (T x) { return x * x; }
//template<class T> inline T min (T x, T y) { return (x < y) ? x : y; }
//template<class T> inline T max (T x, T y) { return (x > y) ? x : y; }

inline float sigmoid (float t) { return atanf(t) / M_PI + 0.5f; }

//----( linear algebra )------------------------------------------------------

struct Matrix
{
  float m_data[9];

  Matrix (float t=0) { for (unsigned i=0; i<9; ++i) m_data[i] = t; }
  Matrix (float a, float b, float c,
          float d, float e, float f,
          float g, float h, float i)
  { m_data[0] = a; m_data[1] = b; m_data[2] = c;
    m_data[3] = d; m_data[4] = e; m_data[5] = f;
    m_data[6] = g; m_data[7] = h; m_data[8] = i; }

  float& data       (int i, int j)       { return m_data[3*i+j]; }
  float  data       (int i, int j) const { return m_data[3*i+j]; }
  float& operator() (int i, int j)       { return data(i,j); }
  float  operator() (int i, int j) const { return data(i,j); }

  void mult (const float* x, float* y) const;
  void mult (const Matrix& m, Matrix& n) const;
  void cleanup ();
  void operator*= (float t) { for (unsigned i=0; i<9; ++i) m_data[i] *= t; }
};

const Matrix identity(1,0,0,
                      0,1,0,
                      0,0,1);

std::ostream& operator<< (std::ostream& os, const Matrix& m)
{
  return os << "\n[["   << m(0,0) << ",\t" << m(0,1) << ",\t" << m(0,2)
            << "],\n [" << m(1,0) << ",\t" << m(1,1) << ",\t" << m(1,2)
            << "],\n [" << m(2,0) << ",\t" << m(2,1) << ",\t" << m(2,2)
            << "]]";
}

// matrix-vector multiplication
void Matrix::mult (const float* x, float* y) const
{
  y[0] = y[1] = y[2] = 0;
  for (unsigned i=0; i<3; ++i) {
    for (unsigned j=0; j<3; ++j) {
      y[i] += operator()(i,j) * x[j];
    } 
  }
}

// matrix-matrix multiplication
void Matrix::mult (const Matrix& m, Matrix& n) const
{
  n *= 0;
  for (unsigned i=0; i<3; ++i)
  for (unsigned j=0; j<3; ++j)
  for (unsigned k=0; k<3; ++k)
    n(i,k) += data(i,j) * m(j,k);
}

void Matrix::cleanup ()
{
  float norm = 0;
  for (unsigned i=0; i<9; ++i) norm += sqr(m_data[i]);
  norm = sqrtf(norm);

  for (unsigned i=0; i<3; ++i) {
    // normalize
    float norm = 0;
    for (unsigned j=0; j<3; ++j) norm += sqr(data(i,j));
    norm = sqrtf(norm);
    for (unsigned j=0; j<3; ++j) data(i,j) /= norm;

    // orthogonalize
    for (unsigned j=0; j<i; ++j) {
      float ip = 0;
      for (unsigned k=0; k<3; ++k) ip += data(i,k) * data(j,k);
      for (unsigned k=0; k<3; ++k) data(i,k) -= ip * data(j,k);
    }
  }

  // rescale
  *this *= norm;
}

//----( viewable map )--------------------------------------------------------

class MapView : public Map
{
  protected:

    struct Point { float rad, eigs[6], pos[3], color[3]; };
    Point* m_points;
    
    typedef std::pair<float, unsigned> Sorted;
    Sorted* m_sorted_verts;
    Sorted* m_sorted_edges;

  public:

    float radius;
    bool draw_verts;
    bool draw_edges;

    MapView ()
      : m_points(NULL),
        m_sorted_verts(NULL),
        m_sorted_edges(NULL),
 
        draw_verts(false),
        draw_edges(true),
        radius(INIT_RAD)
    {}

    void clear ()
    {
      if (m_points) { delete[] m_points; m_points = NULL; }
      if (m_sorted_verts) { delete[] m_sorted_verts; m_sorted_verts = NULL; }
      if (m_sorted_edges) { delete[] m_sorted_edges; m_sorted_edges = NULL; }
    }
    ~MapView() { clear(); }

    void load_map (const char* filename);

    void init_colors();
    void project (const Matrix& m);
    void draw ();
    void reverse_color (int i);

    void rotate_eigs ();
};

void MapView::load_map (const char * filename)
{
  clear();
  Map::load(filename);

  m_points = new Point[m_size];
  m_sorted_verts = new Sorted[m_size];
  m_sorted_edges = new Sorted[m_num_edges];

  for (unsigned v = 0; v < m_size; ++v) {
    Point & point = m_points[v];
    point.rad = fabs(eig(v,0));
    for (unsigned i = 0; i < 6; ++i) {
      point.eigs[i] = eig(v,i+1);
    }
  }

  init_colors();
}

void MapView::init_colors ()
{
  // normalize colors to lie in [0,1]
  const float BIG = 1e30;
  float min_c[3] = {BIG, BIG, BIG};
  float max_c[3] = {-BIG, -BIG, -BIG};
  for (Point *p = m_points, *end = m_points + m_size; p < end; ++p) {
    for (unsigned i = 0; i < 3; ++i) {
      p->color[i] = p->eigs[i+3];
      min_c[i] = std::min(min_c[i], p->color[i]);
      max_c[i] = std::max(max_c[i], p->color[i]);
    }
  }

  float scale_c[3];
  for (unsigned i = 0; i < 3; ++i) {
    scale_c[i] = 1.0 / (max_c[i] - min_c[i]);
  }

  for (Point *p = m_points, *end = m_points + m_size; p < end; ++p) {
    for (unsigned i = 0; i < 3; ++i) {
      p->color[i] -= min_c[i];
      p->color[i] *= scale_c[i];
    }
  }
}

void MapView::project (const Matrix& proj)
{
  for (Point *p = m_points, *end = m_points + m_size; p < end; ++p) {
    proj.mult(p->eigs, p->pos);
  }

  if (draw_verts) {
    for (unsigned v = 0; v < m_size; ++v) {
      Point & point = m_points[v];
      m_sorted_verts[v] = std::make_pair(point.pos[2], v);
    }
    std::sort(m_sorted_verts, m_sorted_verts + m_size);
  }

  if (draw_edges) {
    for (unsigned e = 0; e < m_num_edges; ++e) {
      Edge & edge = m_edges[e];
      float pos = m_points[edge.head].pos[2]
                + m_points[edge.tail].pos[2];
      m_sorted_edges[e] = std::make_pair(pos, e);
    }
    std::sort(m_sorted_edges, m_sorted_edges+m_num_edges);
  }
}

void MapView::draw ()
{
  if (draw_verts) {
    for (unsigned i = 0; i < m_size; ++i) {
      unsigned v = m_sorted_verts[i].second;
      Point & point = m_points[v];

      //draw circle
      float R = radius * point.rad;
      glColor4f(color_map(point.color[0]),
                color_map(point.color[1]),
                color_map(point.color[2]), VERT_OPACITY);
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glBegin(GL_POLYGON);
      const float ds = 2.0 * M_PI / POLY_SIDES;
      for (unsigned s=0; s<=POLY_SIDES; ++s) {
        glVertex3f(point.pos[0] + R * cosf(s * ds),
                   point.pos[1] + R * sinf(s * ds),
                   point.pos[2]);
      }
      glEnd();
    }
  }

  if (draw_edges) {
    glBegin(GL_LINES);
    for (unsigned i = 0; i < m_num_edges; ++i) {
      unsigned e = m_sorted_edges[i].second;
      Edge & edge = m_edges[e];
      Point & head = m_points[edge.head];
      Point & tail = m_points[edge.tail];

      glColor4f(color_map(head.color[0]),
                color_map(head.color[1]),
                color_map(head.color[2]),
                EDGE_OPACITY);
      glVertex3fv(head.pos);

      glColor4f(color_map(tail.color[0]),
                color_map(tail.color[1]),
                color_map(tail.color[2]),
                EDGE_OPACITY);
      glVertex3fv(tail.pos);
    }
    glEnd();
  }
}

void MapView::reverse_color (int i)
{
  for (unsigned v = 0; v < m_size; ++v) {
    reverse(m_points[v].color[i]);
  }
}

void MapView::rotate_eigs ()
{
  for (Point *p = m_points, *end = m_points + m_size; p < end; ++p) {
    float e0 = p->eigs[0];
    for (unsigned i = 0; i < 5; ++i) {
      p->eigs[i] = p->eigs[i+1];
    }
    p->eigs[5] = e0;
  }

  init_colors();
}

MapView map;  // global instance

//----( view control )--------------------------------------------------------

inline void update () { glutPostRedisplay(); }

int width, height;
float x_scale, y_scale;
Matrix proj;
void zoom (float scale)
{
  proj *= scale;
  map.radius *= scale;
  update();
}
float convert_x (float x) { return x_scale * (2.0f * x / width - 1.0f); }
float convert_y (float y) { return y_scale * (1.0f - 2.0f * y / height); }
float drag_x = 0.0f, drag_y = 0.0f;
void mouse_motion (int X, int Y)
{
  float new_x = convert_x(X), x = new_x - drag_x; drag_x = new_x;
  float new_y = convert_y(Y), y = new_y - drag_y; drag_y = new_y;
  float scale = sqrtf(x*x + y*y);
  float angle = ANGLE * scale;
  float c = cosf(angle), s = sinf(angle);
  float i = -y / scale;
  float j = x / scale;

  Matrix rot( i*i*(1-c) + c,  i*j*(1-c),      j*s,
              j*i*(1-c),      j*j*(1-c) + c,  -i*s,
              -j*s,           i*s,            c);

  Matrix rot_proj;
  rot.mult(proj, rot_proj);
  proj = rot_proj;
  //proj.cleanup();

  update();
}

//----( glut callbacks )------------------------------------------------------

void display ()
{
  //clear buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  map.project(proj);
  map.draw();

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

int parse_num ()
{
  std::cout << "number? ";
  unsigned num;
  std::cin >> num;
  return num;
}

void keyboard (unsigned char key, int w_x, int w_y)
{
  switch (key) {
    case ESCKEY:
      exit(0);

    case '-':
    case '_':
      map.radius /= 1.2;
      break;

    case '=':
    case '+':
      map.radius *= 1.2;
      break;

    case 'v':
    case 'V':
      map.draw_verts = not map.draw_verts;
      break;

    case 'e':
    case 'E':
      map.draw_edges = not map.draw_edges;
      break;

    case 'r': map.reverse_color(0); break;
    case 'g': map.reverse_color(1); break;
    case 'b': map.reverse_color(2); break;

    case ' ':
      map.rotate_eigs();
      break;

    default: return;
  }
  update();
}

void special_keys (int key, int w_x, int w_y)
{
  switch (key) {
    case GLUT_KEY_UP:   zoom(ZOOM_IN);  break;
    case GLUT_KEY_DOWN: zoom(ZOOM_OUT); break;
  }
}

void mouse (int button, int state, int X, int Y)
{
  static unsigned drag_channels = 0;
  //scrolling controls zoom
  if (button == SCROLL_UP) {
    if (state == GLUT_UP) zoom(ZOOM_IN);
    return;
  } else if (button == SCROLL_DOWN) {
    if (state == GLUT_UP) zoom(ZOOM_OUT);
    return;
  }

  //start dragging
  bool first_down = (drag_channels == 0);
  if (first_down) {
    //begin dragging
    drag_x = convert_x(X);
    drag_y = convert_y(Y);
  }

  //update drag channels
  if (state == GLUT_DOWN) drag_channels |=   1 << button;
  if (state == GLUT_UP)   drag_channels &= ~(1 << button);
}

const char * help_message =
"Eigenvector mapper. Copyright (C) 2010 Fritz Obermeyer. License: GLPv3\n"
"Usage: view_map infile.map\n"
"Controls:\n"
"  +/-    resize vertices\n"
"  v      toggle vertex drawing\n"
"  e      toggle edge drawing\n"
"  r/g/b  reverses a color channel\n"
"  SPACE  cycle through eigenvectors 1-7\n"
"  ESC    exits\n"
;

int main (int argc,char **argv)
{
  // load map
  if (argc < 2) {
    std::cerr << "usage: view_map infile.map" << std::endl;
    return 1;
  }
  const char * infile = argv[1];
  map.load_map(infile);

  std::cout << help_message << std::flush;
  
  //define main window
  glutInit(&argc, argv);

  //set display mode
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );

  //set window size
  glutInitWindowSize(800,600);
  glutInitWindowPosition(50,50);
  glutCreateWindow(argv[0]);
  reshape(800,600);

  //set callbacks
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special_keys);
  glutMouseFunc(mouse);
  glutMotionFunc(mouse_motion);

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
  glEnable(GL_DEPTH_TEST);

  //init projection matrix
  proj = identity;
  proj *= INIT_ZOOM;

  //set title
  static char title_string[256];
  sprintf(title_string,
          "mapping %u vertices, %u edges", map.num_verts(), map.num_edges());
  glutSetWindowTitle(title_string);

  //give control to glut
  glutMainLoop();

  return 0;
}

