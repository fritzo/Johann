
#include "console.h"
#include <fstream>

//i/o
bool g_interactive_input;
bool g_interactive_output;
bool g_quiet, g_quietly;
bool g_skimming;
void set_quietly (bool quietly) { g_quiet = g_quietly = quietly; }
ostream& j_output () { return std::cout; }
ostream& j_debug () { return std::cout << deb_prompt; }
ostream& j_info ()
{
    static ofstream null_out("/dev/null");
    if (g_quiet) return null_out;
    else         return std::cout << out_prompt;
}
ostream& j_error (int lines_delayed)
{
    if (g_quiet) {
        int error_line = get_line_number() - lines_delayed;
        return std::cout << "error, line " << error_line << ": ";
    } else {
        return std::cout << err_prompt;
    }
}
ostream& j_warning (int lines_delayed)
{
    if (g_quiet) {
        int error_line = get_line_number() - lines_delayed;
        return std::cout << "warning, line " << error_line << ": ";
    } else {
        return std::cout << warn_prompt;
    }
}

//context stack
void ContextStack::clear ()
{
    while (not empty()) std::stack<ContextType>::pop();
}
const char* contextSymb[7] = {"  ","()","[]","<>","{}","\"\"","\\n"};
void ContextStack::pop(ContextType t)
{
    if (empty()) {
        j_error() << "extra closing " << contextSymb[t]
                  << " at EOL" << std::endl;
        return;
    } if (top() != t) {
        j_error() << "extra closing " << contextSymb[t]
                  << " inside " << contextSymb[top()] << std::endl;
        return;
    }
    std::stack<ContextType>::pop();
}
ContextStack contextStack;
void pause_EOL  () { if (contextStack.empty()) contextStack.push(NoEOL); }
void resume_EOL () { if (contextStack.top() == NoEOL) contextStack.clear(); }
StartStack _default_stack;
StartStack* startStack = &_default_stack;

