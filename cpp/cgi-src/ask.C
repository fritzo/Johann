
#include "cgi_tools.h"

using namespace CgiTools;

//globals
string jinput, joutput;
stringstream jin, jout;

int main (void)
{
    //process command
    front_matter("Johann's Library");   //write header
    navigator("ask");                   //navigator

    later();

    back_matter();          //close document

    return 0;
}

