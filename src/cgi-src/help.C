
#include "cgi_tools.h"

using namespace CgiTools;

int main (void)
{
    front_matter("Johann Documentation");   //write header
    navigator("help", true);                //navigator
    insert_html("template/help.html");      //read contents from template
    back_matter(true);                      //close document

    return 0;
}

