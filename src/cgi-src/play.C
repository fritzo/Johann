
#include "../definitions.h"
#include "cgi_tools.h"
#include "../socket_tools.h"

using namespace CgiTools;
using namespace SocketTools;

//globals
string in_str;
unsigned r_steps, p_steps;
string steps = "0";

stringstream jin;  //contents of the editor  window (orange)
stringstream jout; //contents of the comments window (green)

inline bool valid_user () { return get_user() == "fritz";  }

//commands
void start ()
{
    jin << "# write some code here\n"
        << "# then play with it using the tools below\n";
}
#define TYPE(fun) void fun (Socket& client)
TYPE( transform )
{
    client.write(steps);    //steps
    client.write(in_str);   //input
    jin  << client.read();  //result
    jout << client.read();  //errors
}
TYPE( property )
{
    client.write(in_str);   //input
    jin  << in_str;         //echo
    jout << client.read();  //result
}
TYPE( file )
{
    client.write(get_form("file")); //input
    jin  << client.read();          //result
    jout << client.read();          //errors
}
TYPE( status )
{
    jin  << in_str;         //echo
    jout << client.read();  //result
}
#undef TYPE

//dispatch
#define CASE(name,fun) if (command == name) {   \
        client.write(command);                  \
        fun(client);                            \
        client.write("exit");                   \
        return;                                 \
    }
void run_command (string command)
{
    logger.info() << "Running " << command |0;
    Logging::IndentBlock block;

    //open client
    Client client("/tmp/johann_socket");
    if (not client) { jout << EE << "could not connect to server\n"; return; }

    if (command == "reduce") steps = _2string(r_steps);
    if (command == "pretty") steps = _2string(p_steps);

    CASE( "simplify",   transform )
    CASE( "reduce",     transform )
    CASE( "comb",       transform )
    CASE( "pretty",     transform )
    CASE( "echo",       transform )

    CASE( "size" ,      property )
    CASE( "basis" ,     status )
    CASE( "atoms" ,     status )
    CASE( "lang" ,      status )

    CASE( "libs" ,      status )
    CASE( "view" ,      file )
    //CASE( "lib_ld" ,    status )
    CASE( "lib_up" ,    status )

    if (valid_user()) {

    CASE( "stats" ,     status )
    CASE( "update" ,    status )
    CASE( "load" ,      status )
    CASE( "save" ,      status )

    }

    //default is to echo
    jin << in_str;
    jout << "unknown command: " << command;
}
#undef CASE

//diagnostics
std::vector<string> get_libs ()
{
    Client client("/tmp/johann_socket");
    client.write("libs");
    std::vector<string> result = split(client.read());
    client.write("exit");
    return result;
}

int main (void)
{
    Logging::title("player");

    //get state
    in_str  = get_form("input");
    r_steps = string2int(get_form("r_steps"),64);
    p_steps = string2int(get_form("p_steps"),0);
    //logger.debug() << "r_steps = "<< r_steps <<" = "<< get_form("r_steps") |0;
    //logger.debug() << "p_steps = "<< p_steps <<" = "<< get_form("p_steps") |0;

    //process command
    std::string command = get_form("command");
    if (command.empty()) start();
    else run_command(command);

    //find libs
    std::vector<string> libs = get_libs();

    //write header
    front_matter("Johann Editor");  //write header
    navigator("play");              //navigator

    begin_form();

        console(jin.str()); //input window

        //menu
        Grunt grnt;
        //grnt.menu("view")
        //    .but("+", "document.form.editor.rows*=2;")
        //    .but("-", "document.form.editor.rows/=2;")
        //;
        grnt.menu(  "expressions")
            .but(   "simplify", "Send('simplify')")
            .but(   "run",      "Send('reduce')")
            .sel_pow2( "r_steps", 512, r_steps,     "max steps while running")
            .sep()
            .but(   "comb",     "Send('comb')",     "compile to a combinator")
            .but(   "pretty",   "Send('pretty')",   "decompile to a pretty term")
            .sel_pow2( "p_steps", 512, p_steps,     "max copies while decompiling")
            .sep()
            .but(   "size",     "Send('size')",     "size of an expression")
            .but(   "basis",    "Run('basis')",     "show current language")
        ;

        if (not libs.empty()) {
        grnt.menu(  "modules")
            .but(   "view",     "File('view')",     "view selected module")
            .sel_strings("file", libs)
            //.but(   "all",      "Run('lib_ldp')",    "load all modules")
            .but(   "update",   "Run('lib_up')",    "update modules")
        ;
        }

        if (valid_user()) {
        grnt.menu(  "server")
            .but(   "stats",    "Run('stats')",     "server statistics")
            .but(   "save",     "Run('save')",      "save server database")
            .but(   "load",     "Run('load')",      "load server database")
            .but(   "update",   "Run('update')",    "update server database")
        ;
        }
        grnt.keypad(cout);

    end_form();

    message(jout.str());    //message window

    back_matter();          //close document

    return 0;
}

