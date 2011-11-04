
#include <iostream>
#include <execinfo.h>
#include <sys/resource.h> // in /usr/include/sys/

#ifndef DEBUG_LEVEL
  #define DEBUG_LEVEL 0
#endif

#define DEBUG_BACKTRACE (DEBUG_LEVEL > 1)

void _print_backtrace ();
inline void print_backtrace ()
{
    if (DEBUG_BACKTRACE) _print_backtrace( );
}
void _print_backtrace ()
{
    static const size_t max_frames = 50;
    void *frames[max_frames];

    size_t num_frames = backtrace (frames, max_frames);
    char **names = backtrace_symbols (frames, num_frames);

    std::cout << "\nbacktrace " << num_frames << " frames on stack:";
    for (size_t i = 0; i < num_frames; ++i) {
        std::cout << "\n  " << names[i];
    }

    free (names);
}

void print_resources ()
{
    struct rusage results;
    getrusage(RUSAGE_SELF, &results);

    std::cout << "\nProcess resources:";
    std::cout << "\n  system time:        " << results.ru_stime.tv_sec << "sec";
    std::cout << "\n  user time:          " << results.ru_utime.tv_sec << "sec";
    std::cout << "\n  max mem used:       " << results.ru_maxrss << "K";
    std::cout << "\n  major page faults:  " << results.ru_majflt;
    std::cout << "\n  minor page faults:  " << results.ru_minflt;
}

void test_fun ()
{
    print_backtrace();
}

int main ()
{
    print_backtrace();
    test_fun();
    print_resources();

    std::cout << std::endl;
    return 0;
}


