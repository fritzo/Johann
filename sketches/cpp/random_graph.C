
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <cstdlib> //for posix memalign
#include <malloc.h> //for mallinfo
#include <unistd.h> //for getpagesize and sysinfo
#ifdef __GNUC__
    #include <sys/sysinfo.h> //for get_{av}phys_pages
#endif
#define VERTEX_DEGREE 6

void mem_stats ()
{
    //totals
#ifdef __GNUC__
    long pagesize = getpagesize();
    long pages    = get_phys_pages();
    long av_pages = get_avphys_pages();
#else
    long pagesize = sysinfo(_SC_PAGESIZE);
    long pages    = sysinfo(_SC_PHYS_PAGES);
    long av_pages = sysinfo(_SC_AVPHYS_PAGES);
#endif
    std::cout << av_pages << "/" << pages << " pages of size " << pagesize << " available" << std::endl;

    //usage
    struct mallinfo info = mallinfo();
    std::cout << "  malloc allocated " << info.uordblks << " chunks" << std::endl;
    std::cout << "  there are " << info.ordblks << " unused chunks" << std::endl;
    std::cout << "  sbrk allocated " << info.arena << " bytes" << std::endl;
    std::cout << "  mmap allocated " << info.hblks << " chunks = "
              << info.hblkhd << " bytes" << std::endl;
}

inline float sqr (float x) { return x*x; }

struct Vertex
{
    Vertex* neighbors[VERTEX_DEGREE];
    float value, old;
};

long g_num_verts = 0;
Vertex* g_graph = NULL;

inline Vertex* get_random_node ()
{
    return g_graph + (rand() % g_num_verts);
}
inline float get_random_weight ()
{
    return (2.0f*rand()) / float(RAND_MAX) - 1.0;
}

void set_up_graph (long order)
{
    g_num_verts = order;
    posix_memalign((void**)(&g_graph), sizeof(Vertex), sizeof(Vertex)*g_num_verts);
    //g_graph = (Vertex*) malloc(sizeof(Vertex)*g_num_verts);
    //g_graph = (Vertex*) calloc(sizeof(Vertex),g_num_verts);
    mem_stats();
    std::cout << "initializing data" << std::endl;
    for (long v=0; v < g_num_verts; ++v) {
        Vertex &vertex = g_graph[v];
        for (long i=0; i<VERTEX_DEGREE; ++i) {
            //vertex.neighbors[i] = get_random_node();
            vertex.neighbors[i] = g_graph + ((1+i+v) % g_num_verts);
        }
        vertex.old = 0;
        //vertex.value = get_random_weight();
        vertex.value = (v % 129)/64.5 - 1.0;
    }
}

float diffuse ()
{
    static const float diffusion = 1.0f / float(1+VERTEX_DEGREE);
    float stepsize2 = 0.0f;
    for (long v=0; v<g_num_verts; ++v) {
        Vertex &vertex = g_graph[v];
        vertex.old = vertex.value;
        vertex.value *= diffusion;
    }
    for (long v=0; v<g_num_verts; ++v) {
        Vertex &vertex = g_graph[v];
        const float old = diffusion * vertex.old;
        for (long i=0; i<VERTEX_DEGREE; ++i) {
            vertex.neighbors[i]->value += old;
        }
    }
    for (long v=0; v<g_num_verts; ++v) {
        Vertex &vertex = g_graph[v];
        stepsize2 += sqr(vertex.old - vertex.value);
    }
    return sqrt(stepsize2 / g_num_verts);
}

int main (int argc, char** argv)
{
    long order = 20; //default
    if (argc > 1) { order = atoi(argv[1]); }
    std::cout << "defining random graph with 2^" << order << " vertices." << std::endl;
    set_up_graph(1<<order);
    
    float stepsize;
    do {
        std::cout << float(clock())/CLOCKS_PER_SEC << "\tdiffusing, ";
        stepsize = diffuse ();
        std::cout << "stepsize = " << stepsize << std::endl;
    } while (stepsize > 1e-6);
}

