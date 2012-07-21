
#include "util.hpp"
#include <sys/time.h>
#include <vector>

// for log_stack_trace
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

// for stack_trace_abort
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>


namespace pomagma
{

//----------------------------------------------------------------------------
// Logging

inline const char * getenv_default(const char * key, const char * default_val)
{
    const char * result = getenv(key);
    return result ? result : default_val;
}

inline int getenv_default(const char * key, int default_val)
{
    const char * result = getenv(key);
    return result ? atoi(result) : default_val;
}

const char * Log::s_log_filename =
    getenv_default("POMAGMA_LOG_FILE", "pomagma.log");
std::ofstream Log::s_log_stream(s_log_filename, std::ios_base::app);

const unsigned Log::s_log_level(getenv_default("POMAGMA_LOG_LEVEL", 1));

// from "So you want a stand-alone function that prints a stack trace"
// http://stackoverflow.com/questions/4636456/stack-trace-for-c-using-gcc/4732119#4732119

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

void Log::stack_trace_abort ()
{
    void * array[10];
    size_t size;
    char ** strings;
    size_t i;

    size = backtrace(array, 50);
    strings = backtrace_symbols(array, size);

    {
        Log log(0);
        log << "Stack trace:\n";
        for (i = 0; i < size; i++) {
           log << strings[i] << "\n";
        }
    }

    free(strings);

    abort();
}

//----------------------------------------------------------------------------
// Time

timeval g_begin_time;
const int g_init_time __attribute__((unused)) (gettimeofday(&g_begin_time, NULL));
float get_elapsed_time ()
{
    timeval current_time;
    gettimeofday(& current_time, NULL);
    return current_time.tv_sec - g_begin_time.tv_sec +
        (current_time.tv_usec - g_begin_time.tv_usec) * 1e-6;
}

std::string get_date (bool hour)
{
    const size_t size = 20; // fits e.g. 2007:05:17:11:33
    static char buff[size];

    time_t t = time(NULL);
    tm T;
    gmtime_r (&t,&T);
    if (hour) strftime(buff,size, "%Y:%m:%d:%H:%M", &T);
    else      strftime(buff,size, "%Y:%m:%d", &T);
    return buff;
}

} // namespace pomagma
