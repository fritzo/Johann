#ifndef JOHANN_CGI_TOOLS_H
#define JOHANN_CGI_TOOLS_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

#define CONSOLE_WIDTH   80
#define CONSOLE_HEIGHT  24
#define MESS_WIDTH      60
#define MESS_HEIGHT     16

//string tools
namespace CgiTools
{

using namespace cgicc;
using namespace std;

//globals
const string UU = "(U) ", JJ = "(J) ", EE = "(!) ";
extern Cgicc cgi;

//string stuff
int count_lines (const string& s, unsigned max_lines=12);
void join_lines (string& s);
std::vector<string> split (string s);
inline bool isspace (char c)
{ return c == ' ' or c == '\t' or c == '\n' or c == '\r'; }

//boilerplate ends
void front_matter (string my_title);
void navigator (string here, bool bar=false);
void back_matter (bool bar=false);

//templates
void insert_html (string filename);
inline void later () { cout << "<b class=xxx>LATER</b>\n"; }

//input data
inline string get_user () { return cgi.getEnvironment().getRemoteUser(); }
string get_form (string name);
void show_form ();

//gui stuff
void end_form();
void begin_form();
void console (string text, int height=CONSOLE_HEIGHT, int width=CONSOLE_WIDTH);
void message (string text, int height=MESS_HEIGHT, int width=MESS_WIDTH);
void alert (string message);

//point-and-grunt interface
class Widget
{
public:
    virtual void write (ostream& o) const = 0;
    virtual ~Widget () {}
};
class Sep : public Widget
{
public:
    virtual void write (ostream& o) const;
    virtual ~Sep () {}
};
class Button : public Widget
{
    string name, action, title;
public:
    Button(const string& n) : name(n) {}
    Button(const string& n, const string& a) : name(n), action(a) {}
    Button(const string& n, const string& a, const string& t)
        : name(n), action(a), title(t) {}
    virtual ~Button () {}
    virtual void write (ostream& o) const;
};
class Selector : public Widget
{
    string name, title;
    std::vector<string> options;
    string start;
public:
    Selector(const string& n) : name(n) {}
    Selector(const string& n, const string& t) : name(n), title(t) {}
    virtual ~Selector () {}
    Selector& add (string option) { options.push_back(option); return *this; }
    Selector& sel (string s) { start = s; return *this; }
    virtual void write (ostream& o) const;
};
class Menu
{
    string name, title;
    vector<Widget*> widgets;
public:
    Menu () {}
    Menu (const string& n) : name(n) {}
    Menu (const string& n, const string& t) : name(n), title(t) {}
    //~Menu (); lazy: don't delete widgets
    void add (Widget* w) { widgets.push_back(w); }

    //rendering
    void menu (ostream& o) const;
    void keypad (ostream& o) const;
};
class Grunt
{
    vector<Menu> menus;
public:
    Grunt () {}

    //menus
    Grunt& menu ()
        { menus.push_back(Menu()); return *this; }
    Grunt& menu (string name)
        { menus.push_back(Menu(name)); return *this; }
    Grunt& menu (string name, string title)
        { menus.push_back(Menu(name,title)); return *this; }

    //widgets
    Menu& back () { if (menus.empty()) menu("menu"); return menus.back(); }
    Grunt& add (Widget* w) { back().add(w); return *this; }

    //separator
    Grunt& sep () { return add(new Sep); }

    //buttons
    Grunt& but (string name)
        { return add(new Button(name)); }
    Grunt& but (string name, string action)
        { return add(new Button(name,action)); }
    Grunt& but (string name, string action, string title)
        { return add(new Button(name,action,title)); }

    //selectors
    Grunt& sel_strings (string name, std::vector<string> opts);
    Grunt& sel_pow2 (string name, unsigned ub, unsigned start, string title);

    //rendering
    void menu (ostream& o) const;
    void keypad (ostream& o) const;
};

}

#endif
