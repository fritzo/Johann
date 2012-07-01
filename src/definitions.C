
#include "definitions.h"
//#include <ctime>
#include <sys/time.h>
#include <vector>
#include <sys/resource.h> //for rusage
#include <unistd.h> //for isatty

/** \mainpage
 * most of the documentation is in the code, not doxygen.
 * see source files for details.
 */

Int powi (Int x, Int y)
{
    Int result = 1;
    for (Int i=0; i<y; ++i) result *= x;
    return result;
}

bool random_bit ()
{
    static unsigned buffer=0, mask=0;
    mask >>= 1;
    if (not mask) {
        buffer = lrand48();
        mask = 1<<30;
    }
    return buffer & mask;
}

bool compare_nocase::operator() (const string& s, const string& t) const
{
    for (unsigned i=0, I=min(s.length(), t.length()); i<I; ++i) {
        char si = tolower(s[i]);
        char ti = tolower(t[i]);
        if (si < ti) return true;
        if (si > ti) return false;
    }
    if (s.length() < t.length()) return true;
    if (s.length() > t.length()) return false;
    return s < t;
}

//================================ debugging ================================

//validation
bool g_valid = true;
void start_validating () { g_valid = true; }
void decide_invalid   () { g_valid = false; }
bool everything_valid () { return g_valid; }

//================================ i/o ================================

bool is_input_interactive () { return isatty(0); }
bool is_output_interactive () { return isatty(1); }

//================================ logging ================================

namespace Logging
{
ofstream logFile(LOG_FILE, std::ios_base::app);
void switch_to_log (string filename)
{
    logFile.close();
    string log_file = "log/";
    log_file.append(filename);
    logFile.open(log_file.c_str(), std::ios_base::app);
    if (not logFile.is_open()) {
        logFile.open (LOG_FILE, std::ios_base::app);
        logger.warning() << "could not open log file " << filename |0;
    }
}

//title/section label
void title (string name)
{
    live_out << "\033[32m================ "
             << name << " " << get_date()
             << " ================\033[37m" |0; //green
}

//indentation stuff
int indentLevel(0);

//time measurement
timeval g_begin_time, g_current_time;
const int g_time_is_available(gettimeofday(&g_begin_time, NULL));
inline void update_time () { gettimeofday(&g_current_time, NULL); }
inline float elapsed_time ()
{
    update_time();
    float result = g_current_time.tv_sec - g_begin_time.tv_sec;
    static const int res = 10; //in milliseconds
    result += (res*1e-6) * ((g_current_time.tv_usec - g_begin_time.tv_usec)/res);
    return result;
}

//log channels
string fill_8 (string s)
{
    while (s.size() < 8) s.push_back(' ');
    return s;
}
Logger::Logger (string name, LogLevel level)
        : m_name(fill_8(name)), m_level(level)
{}
const string levelBeg[6] =
{
    //these set the color, write the log level, and backspace to write over;
    //  this communicates the log level as color in ansi terminals,
    //  and as a string usable by grep and non-ansi terminals
    "\e[7;31merror   \e[8D",  // error   - reverse red
    "\e[31mwarning \e[8D",    // warning - red
    "\e[35minvalid \e[8D",    // invalid - magenta
    "\e[32minfo    \e[8D",    // info    - green
    "\e[33mdebug   \e[8D"     // debug   - yellow
/*
    "\e[7;31m",  // error   - reverse red
    "\e[31m",    // warning - red
    "\e[35m",    // invalid - magenta
    "\e[32m",    // info    - green
    "\e[33m"     // debug   - yellow
*/
};
const string levelEnd = "\e[0;39m";
const fake_ostream& Logger::active_log (LogLevel level) const
{
    return live_out << elapsed_time() << '\t'
                    << levelBeg[level] << m_name << levelEnd
                    << indentation();
}

}

//time
float get_elapsed_time () { return Logging::elapsed_time(); }
string get_date (bool hour)
{
    const size_t size = 20; //fits e.g. 2007:05:17:11:33
    static char buff[size];

    time_t t = time(NULL);
    tm T;
    gmtime_r (&t,&T);
    if (hour) strftime(buff,size, "%Y:%m:%d:%H:%M", &T);
    else      strftime(buff,size, "%Y:%m:%d", &T);
    return buff;
}

//================================ debugging ================================

void print_resources ()
{
    struct rusage results;
    getrusage(RUSAGE_SELF, &results);

    Logging::logger.debug() << "Process resources:"
        << "\n  system time:        " << results.ru_stime.tv_sec << "sec"
        << "\n  user time:          " << results.ru_utime.tv_sec << "sec"
        << "\n  max mem used:       " << results.ru_maxrss << "K"
        << "\n  major page faults:  " << results.ru_majflt
        << "\n  minor page faults:  " << results.ru_minflt
        |0;
}

