
#include <iostream>
#include <cstdlib>
#include <cstdio>

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

using namespace cgicc;
using std::cout;
using std::cerr;
using std::endl;

int main (void)
{
    // Output the content type and doc type
    cout << HTTPHTMLHeader() << endl;
    cout << HTMLDoctype(HTMLDoctype::eStrict) << endl;
    cout << html().set("lang", "en").set("dir", "ltr") << endl;

    //read input
    Cgicc cgi;
    form_iterator name = cgi.getElement("code");
    std::string input = (name == cgi.getElements().end()) ? "(nothing)"
                                                          : **name;

    //write output
    cout << html() << head(title("Test"))
         << body()
         << p() << "You typed:\n"
         << p() << input
         << body()
         << html(); 

    return 0;
}

