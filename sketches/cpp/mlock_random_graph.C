
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <sys/mman.h> //for mlock
#include <errno.h> //for errno
#define VERTEX_DEGREE 6

inline float sqr (float x) { return x*x; }

struct Vertex
{
    Vertex* neighbors[VERTEX_DEGREE];
    float value, old;
};

int g_num_verts = 0;
Vertex* g_graph = NULL;

inline Vertex* get_random_node ()
{
    return g_graph + (random() % g_num_verts);
}
inline float get_random_weight ()
{
    return (2.0f*random()) / float(RAND_MAX) - 1.0;
}

void set_up_graph (int order)
{
    g_num_verts = order;
    g_graph = new Vertex[g_num_verts];
    
    //lock array
    if (mlock(g_graph, g_num_verts * sizeof(Vertex))) {
        std::cout << "mlock failed: ";
        switch (errno) {
            case ENOMEM: std::cout << "locked page limit exceeded"; break;
            case EPERM:  std::cout << "calling process is not superuser"; break;
            case EINVAL: std::cout << "tried to lock nonpositive range"; break;
            case ENOSYS: std::cout << "kernel does not provide mlock capability"; break;
        }
        std::cout << std::endl;
    }
    
    //initialize data
    for (int v=0; v < g_num_verts; ++v) {
        Vertex &vertex = g_graph[v];
        for (int i=0; i<VERTEX_DEGREE; ++i) {
            vertex.neighbors[i] = get_random_node();
        }
        vertex.old = 0;
        vertex.value = get_random_weight();
    }
}

float diffuse ()
{
    static const float diffusion = 1.0f / float(1+VERTEX_DEGREE);
    float stepsize2 = 0.0f;
    for (int v=0; v<g_num_verts; ++v) {
        Vertex &vertex = g_graph[v];
        vertex.old = vertex.value;
        vertex.value *= diffusion;
    }
    for (int v=0; v<g_num_verts; ++v) {
        Vertex &vertex = g_graph[v];
        const float old = diffusion * vertex.old;
        for (int i=0; i<VERTEX_DEGREE; ++i) {
            vertex.neighbors[i]->value += old;
        }
    }
    for (int v=0; v<g_num_verts; ++v) {
        Vertex &vertex = g_graph[v];
        stepsize2 += sqr(vertex.old - vertex.value);
    }
    return sqrt(stepsize2);
}

int main (int argc, char** argv)
{
    //alternative memory locking function
    if (mlockall(MCL_CURRENT | MCL_FUTURE)) {
        std::cout << "mlockall failed";
        std::cout << std::endl;
        switch (errno) {
            case ENOMEM: std::cout << "locked page limit exceeded"; break;
            case EPERM:  std::cout << "calling process is not superuser"; break;
            case EINVAL: std::cout << "invalid flags passed to mlockall"; break;
            case ENOSYS: std::cout << "kernel does not provide mlockall capability"; break;
        }
        std::cout << std::endl;
    }
    

    int order = 1<<20; //default
    if (argc > 1) { order = atoi(argv[1]); }
    std::cout << "defining random graph with " << order << " vertices." << std::endl;
    set_up_graph(order);
    float stepsize;
    do {
        std::cout << " " << float(clock())/CLOCKS_PER_SEC << "\t: diffusing, ";
        stepsize = diffuse ();
        std::cout << "stepsize = " << stepsize << std::endl;
    } while (stepsize > 1e-6);
}

