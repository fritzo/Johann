
#include <pthread.h>
#include <queue>
#include <iostream>
#include <fstream>

//threading data
pthread_t       p_input_cmd;
pthread_mutex_t p_queue_mutex,
                p_numCycles_mutex,
                p_data_mutex;

//global data
std::queue<double> g_cmd_queue;
int g_numCycles;
int     g_size;
double* g_data;


//functions
void init_data (int size = 1<<3)
{//allocates arrays and sets all data to zero
    g_size = size;
    g_data = new(std::nothrow) double[size];
    if (g_data == NULL) {
        std::cout << "could not allocate data\n";
        exit(-1);
    }
    for (int i=0; i<g_size; ++i) {
        g_data[i] = 0.0;
    }
}

void* input_cmd (void*)
{//pushes a command on the stack, exits when necc.
    double cmd;
    bool exiting = false;
    while (!exiting) {
        //push new command on queue
        std::cout << "> ";
        std::cin >> cmd;
        pthread_mutex_lock( &p_queue_mutex );
        g_cmd_queue.push(cmd);
        pthread_mutex_unlock( &p_queue_mutex );

        //output status since last command
        pthread_mutex_lock( &p_numCycles_mutex );
        std::cout << g_numCycles << " cycles since last command" << std::endl;
        g_numCycles = 0;
        pthread_mutex_unlock( &p_numCycles_mutex );

        //output system status
        pthread_mutex_lock( &p_data_mutex );
        std::cout << "data =";
        for (int i=0; i<g_size; ++i) {
            std::cout << " " << g_data[i];
        }
        std::cout << std::endl;
        pthread_mutex_unlock( &p_data_mutex );

        //text for exit condition
        if (cmd == 0.0) {
            exiting = true;
            std::cout << "exiting\n";
        }
    }
    pthread_exit(NULL);
}

void process_cmd (double cmd)
{//does a laplace transform on input times
    static const double maxExponent = 2;
    pthread_mutex_lock( &p_data_mutex );
    for (int i=0; i<g_size; ++i) {
        double eps = exp(-(maxExponent* i)/(double(g_size)));
        g_data[i] *= eps;
        g_data[i] += (1.0 - eps) * cmd;
    }
    pthread_mutex_unlock( &p_data_mutex );

    pthread_mutex_lock( &p_numCycles_mutex );
    ++g_numCycles;
    pthread_mutex_unlock( &p_numCycles_mutex );
}

void output_data ()
{//writes data to file posix_threads.out
    std::ofstream file("posix_threads.out");
    for (int i=0; i<g_size; ++i) {
        file << g_data[i] << "\n";
    }
    file.close(); //unnecessary but safe
}

int main ()
{
    //initialize data & mutexes
    init_data();
    g_numCycles = 0;
    pthread_mutex_init( &p_numCycles_mutex, NULL );
    pthread_mutex_init( &p_queue_mutex, NULL );
    pthread_mutex_init( &p_data_mutex, NULL );

    //start input thread
    void* passed_args = NULL;
    int status = pthread_create( &p_input_cmd, NULL,
                                 input_cmd, passed_args);
    if (status){
        std::cout << "ERROR: return code from pthread_create() is "
                  << status << "\n";
         exit(-1);
    }

    //main computation loop
    double cmd = 0.0;
    bool exiting = false;
    while (!exiting) {
        process_cmd (cmd);

        //try to pop new command off queue
        pthread_mutex_lock( &p_queue_mutex );
        if (!g_cmd_queue.empty()) {
            cmd = g_cmd_queue.front();
            g_cmd_queue.pop();

            //text for exit condition
            if (cmd == 0.0) { //exit condition
                exiting = true;
            }
        }
        pthread_mutex_unlock( &p_queue_mutex );
    }

    //clean up mutexes
    pthread_mutex_destroy( &p_data_mutex );
    pthread_mutex_destroy( &p_queue_mutex );
    pthread_mutex_destroy( &p_numCycles_mutex );

    output_data();

    pthread_exit(NULL); //so existing threads can continue
}

