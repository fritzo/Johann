
#include "cgi_tools.h"

using namespace CgiTools;

//globals
string jinput, joutput;
stringstream jin, jout;

//commands
bool start ()
{
    jin << "# write some code here\n"
        << "# then play with it using the tools below\n";
    return true;
}
bool reduce ()
{
    jin << jinput;
    jout << UU << jinput << '\n';
    return true;
}
bool run_command (string cmd)
{
    if (cmd.empty()) return start();
    if (cmd == "reduce")  return reduce();

    jout << EE << "unknown command: " << cmd << '\n';

    return false;
}

int main (void)
{
    //process command
    front_matter("Johann's Library");   //write header
    navigator("read", true);            //navigator
    insert_html("template/lib.html");   //read library index from template
    insert_html("template/read.html");  //read other stuff from template
    back_matter(true);                  //close document

    return 0;
}

