
#include "cgi_tools.h"

using namespace CgiTools;

int main (void)
{
    front_matter("Johann");             //write header
    navigator("Johann",true);           //navigator
    insert_html("template/index.html"); //read contents from template
    back_matter(true);                  //close document

    return 0;
}

