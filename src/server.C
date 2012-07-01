
#include "definitions.h"
#include "expressions.h"
#include "substitute.h"
#include "socket_tools.h"
#include "thread_tools.h"
#include "simple.h"
#include "market.h"
#include "version.h"
#include <cstdlib>
#include <sstream>
#include <map>

namespace EX = Expressions;
namespace S  = Substitute;
namespace ST = SocketTools;
namespace TT = ThreadTools;
using TT::new_thread;
using TT::Lock;

using std::cin;
using std::cout;
using std::endl;
using std::ostringstream;
using std::istringstream;

#define SOCK_NAME "/tmp/johann_socket"

#define LS LOCK_SYNTAX
#define US UNLOCK_SYNTAX

namespace Server
{

const Logging::Logger logger("server", Logging::DEBUG);

//global server
ST::Server* server = NULL;
void* serve (void* talk);

//global simplifier
using Simple::Simple;
Simple simplify;

//global market
using Market::Market;
Market market;

//global library
using S::library;

//======== transform commands: (arg) -> (result,out)
ExprHdl reduce  (ExprHdl e, unsigned s) { return e->as_comb()->reduce(s); }
ExprHdl pretty  (ExprHdl e, unsigned s) { return e->pretty(s); }
ExprHdl comb    (ExprHdl e) { return e->as_comb(); }
#define CASE1(name,fun) if (command == name) e = fun(e);
#define CASE2(name,fun) if (command == name) e = fun(e,s);
void transform (string command, ST::Socket* sock)
{
    logger.debug() << "transforming: " << command |0;
    Logging::IndentBlock block;

    unsigned s = string2int(sock->read());
    logger.debug() << "steps = " << s |0;

    LOCK_SYNTAX
    ExprHdl e = EX::parse(sock->read());
    //DEBUG
    //string s = sock->read();
    //ExprHdl e = EX::parse(s);
    //logger.debug() << '"' << s << '"' |0;

    CASE1 ( "simplify",  simplify )
    CASE2 ( "reduce",    reduce )
    CASE1 ( "comb",      comb )
    CASE2 ( "pretty",    pretty )

    sock->write( e->str() );
    e.clear();
    sock->write( EX::parse_errors );

    UNLOCK_SYNTAX
}
#undef CASE1
#undef CASE2

//======== file commands ========
SubsHdl view (string file) { return library.get_file(file); }
#define CASE(name,fun) if (command == name) subs = fun (file);
void file (string command, ST::Socket* sock)
{
    logger.debug() << "file operation: " << command |0;

    LOCK_SYNTAX
    SubsHdl subs = Subs::id();
    string file = sock->read();

    CASE ( "view",  view );

    if (subs) {
        ostringstream out;
        out << "# " << file << '\n';
        subs->write_to(out);

        sock->write(out.str());
        sock->write("");
    } else {
        sock->write("XXX");
        sock->write(command + " failed");
    }
    subs.clear();
    UNLOCK_SYNTAX
}
#undef CASE

//======== clients answering: (arg,arg) -> (out)
void assume (string, ST::Socket* sock)
{
    //logger.debug() << "assuming simplification" |0;
    //Logging::IndentBlock block;

    LOCK_SYNTAX

    ostringstream out;
    ExprHdl lhs = EX::parse(sock->read(),out);
    ExprHdl rhs = EX::parse(sock->read(),out);

    if (lhs->isBad() or rhs->isBad()) {
        logger.error() << "bad inputs to ans_simple" |0;
        out << "bad expressions\n";
    } else {
        simplify.add(lhs, rhs);
    }
    sock->write(out.str());

    lhs.clear();
    rhs.clear();

    UNLOCK_SYNTAX
}

//======== sending data
void send_lang (string, ST::Socket* sock)
{
    logger.debug() << "sending language" |0;
    Logging::IndentBlock block;

    LOCK_SYNTAX
    sock->write("set app");
    sock->write(_2string(simplify.app()));
    for (Simple::bas_iter i=simplify.bas_begin(); i!=simplify.bas_end(); ++i) {
        if (not *sock) break;
        sock->write("set mass");
        sock->write(i->second.first->str());
        sock->write(_2string(i->second.second));
        sock->read(); //ignore message
    }
    UNLOCK_SYNTAX
}
void send_eqns (string, ST::Socket* sock)
{
    logger.debug() << "sending equations" |0;
    Logging::IndentBlock block;

    LOCK_SYNTAX
    for (Simple::map_iter i=simplify.map_begin(); i!=simplify.map_end(); ++i) {
        if (not *sock) break;
        sock->write("assume");
        sock->write(i->second.first->str());
        sock->write(i->second.second->str());
        sock->read(); //ignore message
    }
    UNLOCK_SYNTAX
}

//======== params: (...) -> (out)

void set_mass (string, ST::Socket* sock)
{
    LOCK_SYNTAX

    ExprHdl e = EX::parse(sock->read());
    float mass = string2float(sock->read());
    sock->write(EX::parse_errors);

    logger.info() << "setting mass of " << e << " to " << mass |0;
    simplify.mass(e,mass);
    e.clear();

    UNLOCK_SYNTAX
}
void set_app (string, ST::Socket* sock)
{
    float mass = string2float(sock->read());

    logger.info() << "setting app to " << mass |0;
    LOCK_SYNTAX
    simplify.app(mass);
    UNLOCK_SYNTAX
}

//======== property commands: (arg) -> (out)

void size (ExprHdl e, ostream& out) { out << "size = " << simplify.size(e); }
#define CASE(name,fun) if (command == name) fun(e,out);
void property (string command, ST::Socket* sock)
{
    logger.debug() << "finding property: " << command |0;
    Logging::IndentBlock block;

    ostringstream out;

    LOCK_SYNTAX
    ExprHdl e = EX::parse(sock->read(),out);

    CASE( "size",   size )

    e.clear();
    UNLOCK_SYNTAX

    sock->write(out.str());
}
#undef CASE

//======== output commands: () -> (out)

//server commands
TT::ThreadSafe<bool> updating(false);
void* _update (void*)
{
    logger.info() << "updating simplifier" |0;
    simplify.update();

    logger.info() << "updating market" |0;
    market.update();

    logger.info() << "done updating." |0;
    updating(false);
    return NULL;
}
void update (ostream& o)
{
    Lock lock(updating);
    if (*updating) { o << "busy." << endl; }
    else { o << "updating." << endl; *updating = true; new_thread(_update); }
}

TT::ThreadSafe<bool> filing(false);
void* _save (void*)
{
    logger.info() << "saving server state" |0;

    ST::Socket* sock = ST::new_ozip("data/default.jsess");
    send_lang ("", sock);
    send_eqns ("", sock);
    delete sock;

    logger.info() << "done saving." |0;
    filing(false);
    return NULL;
}
void* _load (void*)
{
    logger.info() << "loading server state" |0;

    serve(ST::new_izip("data/default.jsess"));

    logger.info() << "done loading." |0;
    filing(false);
    return NULL;
}
void save (ostream& o)
{
    Lock lock(filing);
    if (*filing) { o << "busy." << endl; }
    else { o << "saving." << endl; *filing = true; new_thread(_save); }
}
void load (ostream& o)
{
    Lock lock(filing);
    if (*filing) { o << "busy."    << endl; }
    else { o << "loading." << endl; *filing = true; new_thread(_load); }
}

//status commands
#define DEF(fun) void fun (ostream& out)
DEF (atoms)  { LOCK_SYNTAX EX::write_consts_to(out); out<<endl; UNLOCK_SYNTAX }
DEF (lang)   { simplify.write_lang(out); }
DEF (basis)  { out << "atoms = "; atoms(out); lang(out); }
DEF (libs)   { LOCK_SYNTAX library.list(out);   UNLOCK_SYNTAX }
DEF (lib_up) { LOCK_SYNTAX library.update(out); UNLOCK_SYNTAX }
DEF (lib_ld) { LOCK_SYNTAX library.add_file("all",out); UNLOCK_SYNTAX }
DEF (stats)
{
    simplify.write_stats(out);
    if (market.size()) market.write_stats(out);
    if (library.size()) out << "modules: " << library.size() << endl;
    if (updating()) out << "busy updating." << endl;
    if (filing()) out << "busy filing." << endl;
}
#undef DEF

#define CASE(name,fun) if (command == name) fun(out);
void status (string command, ST::Socket* sock)
{
    logger.debug() << "writing status: " << command |0;
    Logging::IndentBlock block;

    ostringstream out;

    CASE( "atoms",  atoms )
    CASE( "lang",   lang )
    CASE( "basis",  basis )

    CASE( "libs",   libs )
    CASE( "lib_ld", lib_ld )
    CASE( "lib_up", lib_up )

    CASE( "stats",  stats )
    CASE( "update", update )
    CASE( "save",   save )
    CASE( "load",   load )

    sock->write(out.str());
}
#undef CASE

//server control
void import (string file, ostream& out)
{
    LOCK_SYNTAX
    library.add_file(file,out);
    UNLOCK_SYNTAX
}
const char* const in_prompt  = "\e[33m(U)\e[39m ";
const char* const out_prompt = "\e[35m(S)\e[39m ";
#define CASE(name,fun) \
    if (command == name) { fun(cout); continue; } \
    commands += " ";  commands += name;
#define CASE1(name,fun) \
    if (command == name) { string arg; in>>arg; fun(arg,cout); continue; } \
    commands += " ";  commands += name; commands += "(-)";
void* console (void* fname)
{
    ifstream file;
    if (fname) {
        logger.info() << "reading script " << fname |0;
        file.open(static_cast<char*>(fname));
        if (not file) {
            logger.error() << "could not open file " << fname |0;
            ST::Client(SOCK_NAME).write("exit");
            return NULL;
        }
    }
    istream& in = fname ? file : cin;
    string command;
    while (in) {
        command = "exit"; //for ctrl-D'ing out
        if (!fname) cout << in_prompt;
        in >> command;
        if (fname) logger.debug() << command |0;

        if (command.empty()) continue;
        if (command == "exit") {
            if (!fname) cout << out_prompt << "bye!" << endl;
            break;
        }
        string commands = "commands: exit";

        CASE( "atoms",  atoms )
        CASE( "lang",   lang )
        CASE( "basis",  basis )

        CASE( "libs",   libs )
        CASE( "lib_ld", lib_ld )
        CASE( "lib_up", lib_up )
        CASE1("import", import )

        CASE( "stats",  stats )
        CASE( "update", update )
        CASE( "save",   save )
        CASE( "load",   load )

        if (!fname) cout << out_prompt << commands << endl;
    }
    if (fname) file.close();
    else server->stop_serving();
    ST::Client(SOCK_NAME).write("exit");
    return NULL;
}
#undef CASE
#undef CASE1

//the service
#define CASE(name,fun) if (command == name) { fun(name,sock); continue; }
void* serve (void* talk)
{
    ST::Socket* sock = reinterpret_cast<ST::Socket*>(talk);

    while (*sock) {

        string command = sock->read();
        if (command == "exit") break;

        CASE( "echo" ,        transform )
        CASE( "reduce" ,      transform )
        CASE( "simplify" ,    transform )
        CASE( "comb" ,        transform )
        CASE( "pretty" ,      transform )

        CASE( "size" ,        property )
        CASE( "basis" ,       status )
        CASE( "atoms" ,       status )
        CASE( "lang" ,        status )

        CASE( "libs" ,        status )
        CASE( "view" ,        file )
        CASE( "lib_ld" ,      status )
        CASE( "lib_up" ,      status )

        CASE( "stats" ,       status )
        CASE( "update" ,      status )
        CASE( "save" ,        status )
        CASE( "load" ,        status )

        CASE( "assume" ,      assume )
        CASE( "set app" ,     set_app )
        CASE( "set mass" ,    set_mass )

        sock->write("unknown command: " + command);
    }
    delete sock;
    return NULL;
}
#undef CASE

}

using namespace Server;

int main (int argc, char** argv)
{
    //log title
    Logging::switch_to_log("server.log");
    std::ostringstream title;
    title << "Johann Server " << VERSION;
    Logging::title(title.str());

    //write title
    cout << "\e[1mJohann Server " << VERSION << "\e[0m"
         << " copyright (c) 2007-2009 Fritz Obermeyer" << endl;

    //set xterm title
    cout << "\033]0;Johann Server " << VERSION << "\007";

    EX::initialize("_ T I K F B C W S Y J R U V P");

    server = new ST::Server(SOCK_NAME);
    for (int i=1; i<argc; ++i) new_thread(console, argv[i]);
    new_thread(console);
    server->serve(serve);
    delete server;

    simplify.clear();
    market.clear();
    S::library.clear();
    EX::clear();

    return 0;
}


