
#include <iostream>
#include <string>

//#include "cgicc/CgiDefs.h"
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

using namespace cgicc;
using namespace std;

//globals
Cgicc cgi;
HTTPHTMLHeader header;

//cookies
string get_cookie (string name)
{
    const vector<HTTPCookie>& cookies
        = cgi.getEnvironment().getCookieList();
    for (unsigned i=0; i<cookies.size(); ++i) {
        if (cookies[i].getName() == name) return cookies[i].getValue();
    }
    return "";
}
void set_cookie (string name, string value)
{
    header.setCookie(HTTPCookie(name,value));
}

//form data
string get_form (string name)
{
    form_iterator iter = cgi.getElement(name);
    if (iter == cgi.getElements().end()) return "";
    return **iter;
}

//commands
bool restart ()
{
    set_cookie("input", "");
    set_cookie("output", "");

    return true;
}
bool wrap ()
{
    //string input = get_form("input");
    string input = get_cookie("input");
    set_cookie("output", input + input);

    return true;
}
bool run_command (string cmd)
{
    if (cmd == "restart") return restart();
    if (cmd == "eval")    return wrap();

    return false;
}

int main (void)
{
    //process command
    set_cookie("path", "/");
    set_cookie("author", "console.cgi");
    //run_command(get_form("command"));
    run_command(get_cookie("command"));

    //write header
    cout << header;
    cout << HTMLDoctype(HTMLDoctype::eStrict) << endl;

    //write template
    ifstream console_html("../html/console.html");
    cout << console_html.rdbuf() << endl;

    return 0;
}

