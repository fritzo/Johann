/* jmapper, a database visualizer for Johann databases.

This file is part of Johann.
Copyright 2004-2009 Fritz Obermeyer.

Johann is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Johann is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Johann.  If not, see <http://www.gnu.org/licenses/>.
*/

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

#define Error(mess) {std::cout\
    << "\n\t" << mess << "\n\t"\
    << __FILE__ << " : " << __LINE__ << "\n\t"\
    << __PRETTY_FUNCTION__ << std::endl;\
    exit(0);}
#define TODO() {Error("control reached unfinished code:");}
#define Assert(cond,mess) {if (!(cond)) Error(mess);}

#define LABEL_SIZE 256

//keyboard & mouse numbers
#define ENTERKEY    13
#define ESCKEY      27
#define SCROLL_UP   3
#define SCROLL_DOWN 4

#define COLOR_BG    1.0, 1.0, 1.0
#define COLOR_FG    0.0, 0.0, 0.0
#define COLOR_LHS   1.0, 0.4, 0.4
#define COLOR_RHS   0.4, 0.4, 1.0
#define COLOR_P_LHS 0.5, 0.0, 0.0
#define COLOR_P_RHS 0.0, 0.0, 0.5
#define ON_OPACITY  0.4
#define OFF_OPACITY 0.08

#define POLY_SIDES  10
#define LINE_WIDTH  6.0f
#define MIN_WIDTH   1.0f
#define MIN_OPACITY 0.3f

#define INIT_RAD    0.001f

#define INIT_ZOOM   100.0f
#define ZOOM_IN     1.2f
#define ZOOM_OUT    1.0f/1.2f
#define ANGLE       2.0f

#define SIMPLEST    24

typedef uint32_t Int;

template<class T> inline T sqr (T x) { return x * x; }
template<class T> inline T min (T x, T y) { return (x < y) ? x : y; }
template<class T> inline T max (T x, T y) { return (x > y) ? x : y; }

inline float sigmoid (float t) { return atanf(t) / M_PI + 0.5f; }

//global gl stuff
inline void update () { glutPostRedisplay(); }

//matrices
struct Matrix
{
    float data[9];

    Matrix (float t=0) { for (Int i=0; i<9; ++i) data[i] = t; }
    Matrix (float a, float b, float c,
            float d, float e, float f,
            float g, float h, float i)
    { data[0] = a; data[1] = b; data[2] = c;
      data[3] = d; data[4] = e; data[5] = f;
      data[6] = g; data[7] = h; data[8] = i; }

    float& operator() (int i, int j)       { return data[3*i+j]; }
    float  operator() (int i, int j) const { return data[3*i+j]; }
    void mult (const float* x, float* y) const;
    void mult (const Matrix& m, Matrix& n) const;
    void cleanup ();
    void operator*= (float t) { for (Int i=0; i<9; ++i) data[i] *= t; }
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
void Matrix::mult (const float* x, float* y) const
{
    y[0] = y[1] = y[2] = 0;
    for (Int i=0; i<3; ++i) {
        for (Int j=0; j<3; ++j) {
            y[i] += operator()(i,j) * x[j];
        } 
    }
}
void Matrix::mult (const Matrix& m, Matrix& n) const
{
    n *= 0;
    for (Int i=0; i<3; ++i)
    for (Int j=0; j<3; ++j)
    for (Int k=0; k<3; ++k)
        n(i,k) += operator()(i,j) * m(j,k);
}
void Matrix::cleanup ()
{
    float norm = 0;
    for (Int i=0; i<9; ++i) norm += sqr(data[i]);
    norm = sqrtf(norm);

    for (Int i=0; i<3; ++i) {
        //normalize
        float norm = 0;
        for (Int j=0; j<3; ++j) norm += sqr(operator()(i,j));
        norm = sqrtf(norm);
        for (Int j=0; j<3; ++j) operator()(i,j) /= norm;

        //orthogonalize
        for (Int j=0; j<i; ++j) {
            float ip = 0;
            for (Int k=0; k<3; ++k) ip += operator()(i,k) * operator()(j,k);
            for (Int k=0; k<3; ++k) operator()(i,k) -= ip * operator()(j,k);
        }
    }

    //rescale
    *this *= norm;
}

//map object
struct Coord
{
    float comp;
    Int parse;
    float unused, x[3], c[3];

    friend inline std::istream& operator>> (std::istream& is, Coord& c)
    {
        return is >> c.comp >> c.parse >> c.unused
                  >> c.x[0] >> c.x[1] >> c.x[2]
                  >> c.c[0] >> c.c[1] >> c.c[2];
    }
};
struct App
{
    uint16_t a,l,r;

    friend inline std::istream& operator>> (std::istream& is, App& e)
    { return is >> e.a >> e.l >> e.r; }
};
struct Comp
{
    uint16_t c,l,r;

    friend inline std::istream& operator>> (std::istream& is, Comp& e)
    { return is >> e.c >> e.l >> e.r; }
};
enum State { PARSE_DOWN=1, PARSE_LEFT=2, PARSE_RIGHT=4, PARSE=8, LABEL=16 };
struct Point
{
    float r, comp, x[3], c[3];
    int state;
    Point () : state(0) {}
};
struct Map
{
    Int Nobs, Napps, Ncomps;
    float P_app, P_comp;
    Coord* coords;
    Coord lower, upper;
    App* apps;
    Comp* comps;
    Point* points;
    std::vector<Int> a_links;
    std::vector<Int> c_links;
    std::vector<std::string> labels, pretty;
    std::pair<float, Int>* sorted;

    void load (const char* filename);

    void project (const Matrix& m);
    void draw ();

    bool circles;
    bool parsing;
    bool pretty_labels;
    void* font;

    void reverse_color (int i);
    void label_below (Int i);
    void select (Int ob, int flag);
    void select (float x, float y, int flag);
    void update_links ();
    void deselect (Int simplest = SIMPLEST);

    void clear ()
    {
        if (not Nobs) return;
        Nobs = 0;
        Napps = 0;
        Ncomps = 0;
        delete[] coords;
        delete[] apps;
        delete[] comps;
        delete[] points;
        delete[] sorted;
    }

    Map () : Nobs(0), Napps(0), Ncomps(0),
             circles(true), parsing(false), pretty_labels(false),
             font(GLUT_BITMAP_HELVETICA_12)
    {}
    ~Map() { clear(); }
};
Map map; //global instance
void Map::load (const char* filename)
{
    std::cout << "loading map file " << filename << std::endl;

    std::ifstream file(filename);
    Assert(file, "failed to open " << filename);
    std::string section;

    //params
    do { file >> section; } while (section.empty());
    Assert (section == "PARAMS", "bad section: " << section);
    file >> Nobs >> Napps >> Ncomps >> P_app >> P_comp;
    std::cout << "  loading " << Nobs << " points, "
                              << Napps << " apps, "
                              << Ncomps << " comps" << std::flush;

    //coords
    do { file >> section; } while (section.empty());
    Assert (section == "COORDS", "bad section: " << section);
    coords = new(std::nothrow) Coord[Nobs];
    points = new(std::nothrow) Point[Nobs];
    sorted = new(std::nothrow) std::pair<float,Int>[Nobs];
    Assert(coords, "no memory for coords");
    Assert(points, "no memory for points");
    Assert(sorted, "no memory for sorted");
    for (Int i=0; i<Nobs; ++i) file >> coords[i];
    std::cout << "." << std::flush;

    //app equations
    do { file >> section; } while (section.empty());
    Assert (section == "APPS", "bad section: " << section);
    apps = new(std::nothrow) App[Napps];
    Assert(apps, "no memory for equations");
    for (Int i=0; i<Napps; ++i) file >> apps[i];
    std::cout << "." << std::flush;

    //comp equations
    do { file >> section; } while (section.empty());
    Assert (section == "COMPS", "bad section: " << section);
    comps = new(std::nothrow) Comp[Ncomps];
    Assert(comps, "no memory for equations");
    for (Int i=0; i<Ncomps; ++i) file >> comps[i];
    std::cout << "." << std::flush;

    //labels
    do { file >> section; } while (section.empty());
    Assert (section == "LABELS", "bad section: " << section);
    char line[LABEL_SIZE];
    file.getline(line,LABEL_SIZE); //get rid of newline
    for (Int i=0; i<Nobs; ++i) {
        file.getline(line, LABEL_SIZE);
        labels.push_back(line);
    }
    std::cout << "." << std::flush;

    //pretty labels
    do { file >> section; } while (section.empty());
    Assert (section == "PRETTY", "bad section: " << section);
    file.getline(line,LABEL_SIZE); //get rid of newline
    for (Int i=0; i<Nobs; ++i) {
        file.getline(line, LABEL_SIZE);
        pretty.push_back(line);
    }
    std::cout << "." << std::flush;

    file.close();

    //find bounds
    lower = coords[0];
    upper = coords[0];
    for (Int i=1; i<Nobs; ++i) {
        if (not coords[i].comp > 0) continue;
        upper.comp = max(coords[i].comp, upper.comp);
        for (Int k=0; k<3; ++k) {
            lower.c[k] = min(lower.c[k], coords[i].c[k]);
            upper.c[k] = max(upper.c[k], coords[i].c[k]);
        }
    }
    std::cout << "." << std::flush;

    //set colors & radii
    for (Int i=0; i<Nobs; ++i) {
        Coord& coord = coords[i];
        Point& point = points[i];
        point.comp = coord.comp;
        //point.r = 2.0f * sigmoid(400.0f * coord.comp - 2.0f) + 1.0f;
        point.r = 50.0f / sqrtf(1.0f-logf(coord.comp / upper.comp));
        for (Int k=0; k<3; ++k) {
            point.c[k] = (coord.c[k] - lower.c[k])
                       / (upper.c[k] - lower.c[k]);
        }
    }

    //reset drawing
    deselect(0);

    std::cout << "loaded" << std::endl;
}
void Map::project (const Matrix& proj)
{
    for (Int i=0; i<Nobs; ++i) {
        Coord& coord = coords[i];
        Point& point = points[i];
        proj.mult(coord.x, point.x);
    }

    //depth-sort
    for (Int i=0; i<Nobs; ++i) {
        sorted[i] = std::make_pair(points[i].x[2], i);
    }
    std::sort(sorted, sorted+Nobs);
}
float radius = INIT_RAD;
void Map::draw ()
{
    //draw points
    for (Int j=0; j<Nobs; ++j) {
        Int i = sorted[j].second;
        Point& point = points[i];

        //draw circle
        float R = radius * point.r;
        glColor4f(point.c[0], point.c[1], point.c[2],
                  point.state ? 1.0f
                              : circles ? ON_OPACITY
                                        : OFF_OPACITY);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBegin(GL_POLYGON);
        float ds = 2.0 * M_PI / POLY_SIDES;
        for (Int s=0; s<=POLY_SIDES; ++s) {
            glVertex3f(point.x[0] + R * cosf(s * ds),
                       point.x[1] + R * sinf(s * ds),
                       point.x[2]);
        }
        glEnd();

        //draw labels
        if (point.state) {
            glColor4f(COLOR_FG, 1.0f);
            const std::string& label = pretty_labels ? pretty[i] : labels[i];
            R *= 0.7;
            glRasterPos3f(point.x[0] + R,
                          point.x[1] + R,
                          point.x[2] + R);
            for (Int i=0; i<label.size(); ++i) {
                glutBitmapCharacter(font, label[i]);
            }
        }
    }

    //draw app links
    for (Int i=0; i<a_links.size(); ++i) {
        App& eqn = apps[a_links[i]];
        Point &L = points[eqn.l], &R = points[eqn.r], &A = points[eqn.a];

        float z_A = coords[eqn.a].comp;
        float z_L = coords[eqn.l].comp, len_L = -logf(z_L);
        float z_R = coords[eqn.r].comp, len_R = -logf(z_R);
        float part = P_app * z_L * z_R / z_A;
        float width_L = part * len_L / (len_L + len_R);
        float width_R = part * len_R / (len_L + len_R);

        // L --> A
        glLineWidth(max(width_L * LINE_WIDTH, MIN_WIDTH));
        glColor4f(COLOR_LHS, max(width_L, MIN_OPACITY));
        glBegin(GL_LINES);
            glVertex3fv(L.x);
            glVertex3fv(A.x);
        glEnd();

        // R --> A
        glLineWidth(max(width_R * LINE_WIDTH, MIN_WIDTH));
        glColor4f(COLOR_RHS, max(width_R, MIN_OPACITY));
        glBegin(GL_LINES);
            glVertex3fv(R.x);
            glVertex3fv(A.x);
        glEnd();
    }

    //draw comp links
    for (Int i=0; i<c_links.size(); ++i) {
        Comp& eqn = comps[c_links[i]];
        Point &L = points[eqn.l], &R = points[eqn.r], &C = points[eqn.c];

        float z_C = coords[eqn.c].comp;
        float z_L = coords[eqn.l].comp, len_L = -logf(z_L);
        float z_R = coords[eqn.r].comp, len_R = -logf(z_R);
        float part = P_comp * z_L * z_R / z_C;
        float width_L = part * len_L / (len_L + len_R);
        float width_R = part * len_R / (len_L + len_R);

        // L --> C
        glLineWidth(max(width_L * LINE_WIDTH, MIN_WIDTH));
        glColor4f(COLOR_LHS, max(width_L, MIN_OPACITY));
        glBegin(GL_LINES);
            glVertex3fv(L.x);
            glVertex3fv(C.x);
        glEnd();

        // R --> C
        glLineWidth(max(width_R * LINE_WIDTH, MIN_WIDTH));
        glColor4f(COLOR_RHS, max(width_R, MIN_OPACITY));
        glBegin(GL_LINES);
            glVertex3fv(R.x);
            glVertex3fv(C.x);
        glEnd();
    }

    glShadeModel(GL_SMOOTH); //---------------------------------------
    glDisable(GL_DEPTH_TEST);

    //draw parses
    /*XXX TODO update for composition
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    if (parsing and a_links.empty() and c_links.empty()) {
        for (Int i=0; i<Nobs; ++i) {
            App& eqn = apps[coords[i].parse];
            Point &L = points[eqn.l], &R = points[eqn.r], &A = points[eqn.a];
            glColor4f(COLOR_P_LHS, 0.2f);   glVertex3fv(A.x);
            glColor4f(COLOR_P_LHS, 0.0f);   glVertex3fv(L.x);
            glColor4f(COLOR_P_RHS, 0.2f);   glVertex3fv(A.x);
            glColor4f(COLOR_P_RHS, 0.0f);   glVertex3fv(R.x);
        }
    } else {
        for (Int i=0; i<Nobs; ++i) {
            if (not (points[i].state & PARSE)) continue;
            App& eqn = apps[coords[i].parse];
            if (eqn.a == eqn.r or eqn.a == eqn.l) continue; //ignore x=Ix etc.
            Point &L = points[eqn.l], &R = points[eqn.r], &A = points[eqn.a];
            glColor4f(COLOR_P_LHS, 0.5f);   glVertex3fv(A.x);
            glColor4f(COLOR_P_LHS, 0.15f);  glVertex3fv(L.x);
            glColor4f(COLOR_P_RHS, 0.5f);   glVertex3fv(A.x);
            glColor4f(COLOR_P_RHS, 0.15f);  glVertex3fv(R.x);
        }
    }
    glEnd();
    */

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT); //---------------------------------------
}
inline void reverse (float& t) { t = 1.0f - t; }
void Map::reverse_color (int i)
{
    for (Int j=0; j<Nobs; ++j) reverse(points[j].c[i]);
}
void Map::update_links ()
{
    a_links.clear();
    for (Int i=0; i<Napps; ++i) {
        App& eqn = apps[i];
        if ((points[eqn.l].state & PARSE_LEFT)
         or (points[eqn.r].state & PARSE_RIGHT)
         or (points[eqn.a].state & PARSE_DOWN)) a_links.push_back(i);
    }

    c_links.clear();
    for (Int i=0; i<Ncomps; ++i) {
        Comp& eqn = comps[i];
        if ((points[eqn.l].state & PARSE_LEFT)
         or (points[eqn.r].state & PARSE_RIGHT)
         or (points[eqn.c].state & PARSE_DOWN)) c_links.push_back(i);
    }

    update();
}
void Map::label_below (Int i)
{//returns whether state is turned on
    if (points[i].state & PARSE) {
        points[i].state = 0;
        return;
    }
    points[i].state |= PARSE | LABEL;
    std::vector<Int> q(1,i);
    while (not q.empty()) {
        //XXX TODO update for composition
        App& eqn = apps[coords[q.back()].parse];
        q.pop_back();
        if (not (points[eqn.l].state & PARSE)) {
            points[eqn.l].state |= LABEL;
            if (eqn.l >= SIMPLEST) {
                points[eqn.l].state |= PARSE;
                q.push_back(eqn.l);
            }
        }
        if (not (points[eqn.r].state & PARSE)) {
            points[eqn.r].state |= LABEL;
            if (eqn.r >= SIMPLEST) {
                points[eqn.r].state |= PARSE;
                q.push_back(eqn.r);
            }
        }
    }
    circles = false;
}
void toggle_state (int& state)
{
    const int mask = PARSE_DOWN | PARSE_LEFT | PARSE_RIGHT;
    int parse_part = state & mask;
    state &= ~mask;
    switch (parse_part) {
        case PARSE_DOWN:    parse_part = PARSE_LEFT;    break;
        case PARSE_LEFT:    parse_part = PARSE_RIGHT;   break;
        case PARSE_RIGHT:   parse_part = 0;             break;
        default:            parse_part = PARSE_DOWN;
    }
    state ^= parse_part;
    //map.circles = not parse_part;
}
void Map::select (float x, float y, int flag)
{
    bool changed = false;
    for (Int i=0; i<Nobs; ++i) {
        Point& p = points[i];
        if (sqr(p.x[0] - x) + sqr(p.x[1] - y) > sqr(radius * p.r)) continue;
        switch (flag) {
            case GLUT_LEFT_BUTTON:   label_below(i);                break;
            case GLUT_RIGHT_BUTTON:  toggle_state(p.state);         break;
            case GLUT_MIDDLE_BUTTON: p.state ^= LABEL; update();    break;
        }
        changed = true;
    }
    if (changed) update_links();
}
void Map::select (Int i, int flag)
{
    Point& p = points[i];
    switch (flag) {
        case GLUT_LEFT_BUTTON:   label_below(i);        break;
        case GLUT_RIGHT_BUTTON:  p.state |= PARSE_DOWN; break;
        case GLUT_MIDDLE_BUTTON: p.state |= PARSE_LEFT
                                         |  PARSE_RIGHT; break;
    }
    std::cout << "> " << labels[i] << std::endl;
    update_links();
}
void Map::deselect (Int simplest)
{
    //keep showing more labels
    static Int state = 0;
    if (simplest == 0) state = 0;
    else simplest = (state += simplest);

    //clear existing lables
    for (Int i=0; i<Nobs; ++i) {
        Point& p = points[i];
        p.state = 0;
    }
    a_links.clear();
    c_links.clear();

    //turn on labels of SIMPLEST terms
    for (Int i=0; i<simplest; ++i) {
        points[i].state = LABEL;
    }
}

//view control
int width, height;
float x_scale, y_scale;
Matrix proj;
void zoom (float scale)
{
    proj *= scale;
    radius *= scale;
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

//glut callbacks
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
    Int num;
    std::cin >> num;
    return num;
}
void keyboard (unsigned char key, int w_x, int w_y)
{
    switch (key) {
        case ESCKEY: exit(0);                                           break;
        case '-': case '_': radius /= 1.2;                    update(); break;
        case '=': case '+': radius *= 1.2;                    update(); break;
        case ' ': if (map.circles) map.deselect(0);
                  else             map.circles = true;        update(); break;
        case 's': case 'S': map.deselect();                   update(); break;
        case 'c': case 'C': map.circles = not map.circles;    update(); break;
        case 'p': case 'P': map.pretty_labels = not map.pretty_labels;
                                                              update(); break;
        case 'a': case 'A': map.parsing = not map.parsing;    update(); break;

        case '1': map.font = GLUT_BITMAP_HELVETICA_10;        update(); break;
        case '2': map.font = GLUT_BITMAP_HELVETICA_12;        update(); break;
        case '3': map.font = GLUT_BITMAP_HELVETICA_18;        update(); break;

        case 'r': map.reverse_color(0);                       update(); break;
        case 'g': map.reverse_color(1);                       update(); break;
        case 'b': map.reverse_color(2);                       update(); break;

        /*
        case 'u': case 'U': map.select(parse_num(), 1);                 break;
        case 'd': case 'D': map.select(parse_num(), 2);                 break;
        case 'l': case 'L': map.select(parse_num(), 3);                 break;
        case 's': case 'S': map.deselect(parse_num());                  break;
        */
    }
}
void special_keys (int key, int w_x, int w_y)
{
    switch (key) {
        case GLUT_KEY_UP:   zoom(ZOOM_IN);  break;
        case GLUT_KEY_DOWN: zoom(ZOOM_OUT); break;
    }
}
Int drag_channels = 0;
void mouse (int button, int state, int X, int Y)
{
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

    //click on vertices
    if (map.circles and state == GLUT_DOWN) {
        map.select(convert_x(X), convert_y(Y), button);
    }
}

const char* help_message =
"Johann mapper. Copyright (C) 2007 Fritz Obermeyer. License: GLPv3\n\
Usage: jmapper [map file name]\n\
Controls:\n\
  +/-    resize points\n\
  SPACE  clear state\n\
  s      clear state and show simplest labels\n\
  c      toggle circle drawing\n\
  a      toggle parsing of ALL points\n\
  p      toggle printing style\n\
  r/g/b  reverses a color channel";
const char* interactive_message =
"  u ...  parse upwards one step\n\
  d ...  parse downwards one step\n\
  l ...  parse entire tree\n\
  s ...  clear parse";

int main (int argc,char **argv)
{
    std::cout << help_message << std::endl;
    //load map
    if (argc>1) map.load(argv[1]);
    else        map.load("stats/default.map");
    
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
    sprintf(title_string, "mapping %u points, %u apps, %u comps",
            map.Nobs, map.Napps, map.Ncomps);
    glutSetWindowTitle(title_string);

    //give control to glut
    glutMainLoop();

    return 0;
}

