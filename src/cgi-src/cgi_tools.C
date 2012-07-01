
#include "cgi_tools.h"
#include <fstream>

namespace CgiTools
{

//globals
Cgicc cgi;

//string stuff
template<class T>
inline string _2string (T t) { ostringstream s; s << t; return s.str(); }
int count_lines (const string& s, unsigned max_lines)
{
    unsigned lines = 1;
    for (unsigned i=0; i<s.size(); ++i) {
        if (s[i] == '\n') ++lines;
    }
    return (lines > max_lines) ? max_lines : lines;
}
inline bool iswhite (char c)
{
    return c == ' ' or c == '\t' or c == '\n' or c == '\r';
}
void strip (string& s)
{
    int i=0,j=s.size();
    while (i<s.size() and iswhite(s[i])) ++i;
    while (j>i and iswhite(s[j-1])) --j;
    s = s.substr(i,j);
}
void join_lines (string& s)
{//just replace '\n' and '\r' with ' '
    for (unsigned i=0; i<s.size(); ++i) {
        char& c = s[i];
        if (c == '\n' or c == '\r') c = ' ';
    }
}
std::vector<string> split (string s)
{
    enum State { SPACE, CHAR };
    State state = SPACE;
    std::vector<string> result;
    for (unsigned i=0; i<s.size(); ++i) { char c = s[i];
        if (state == SPACE) {
            if (isspace(c)) continue;
            state = CHAR;
            result.push_back("");
        } else {
            if (isspace(c)) { state = SPACE; continue; }
        }
        result.back().push_back(c);
    }
    return result;
}

//boilerplate ends
void front_matter (string my_title)
{
    cout << HTTPHTMLHeader();
    cout << HTMLDoctype(HTMLDoctype::eStrict) << endl;
    cout << html() << head() << '\n';
    cout << title(my_title) << '\n';
    cout << script().set("language","javascript")
                    .set("src","console.js") << script() << '\n';
    cout << cgicc::link().set("rel","stylesheet")
                         .set("type","text/css")
                         .set("href","main.css") << '\n';
    cout << cgicc::link().set("rel","shortcut icon")
                         .set("type","image/vnd.microsoft.icon")
                         .set("href","favicon.ico") << '\n';
    cout << head() << '\n' << endl;
    cout << body() << "\n\n";
}
std::string current_location;
void go_to (string href, string name, string title="", bool live=false)
{
    a e;
    e.set("href",href);
    if (not title.empty()) e.set("title",title);
    if (name == "Johann") e.set("class","main");
    else {
        if (name == current_location) {
            e.set("class","here");
            name = "(" + name + ")";
        }
        cout << " - \n";
    }
    cout << e << name << a();
}
void navigator (string here, bool bar)
{
    current_location = here;
    cout << "<div id=\"navigator\">\n";
    go_to("index.html",     "Johann",   "go to main page");
    go_to("play.html",      "play",     "use the coding tool",      true);
    //go_to("ask.html",       "ask",      "pose a problem",           true);
    //go_to("teach.html",     "teach",    "give a solution",          true);
    go_to("read.html",      "read",     "browse documentation");
    go_to("help.html",      "help");
    //go_to("help#"+here,     "help");
    if (bar) cout << hr() << '\n';
    cout << "\n</div>\n\n";
}
void back_matter (bool bar)
{
    cout << "<div id=\"copyright\">\n";
    if (bar) cout << hr() << '\n';
    cout << "copyright &copy; 2007-2008 "
         << a().set("href","http://www.math.cmu.edu/~fho/index.html")
               .set("target","'_blank'")
         << "fritz obermeyer"
         << a()
         << "</div>\n\n";
    cout << body() << html() << endl;
}

//templates
void insert_html (string filename)
{
    cout << "\n\n<!-- begin inserting file -->\n\n";
    ifstream file(filename.c_str());
    if (file) { cout << file.rdbuf(); file.close(); }
    else      { cout << "ERROR: missing template file"; }
    cout << "\n\n<!-- end inserting file -->\n\n";
}

//input data
string get_form (string name)
{
    form_iterator iter = cgi.getElement(name);
    if (iter == cgi.getElements().end()) return "";
    return **iter;
}
void show_form ()
{
    cout << p() << '\n';
    for (const_form_iterator iter = cgi.getElements().begin();
            iter != cgi.getElements().end(); ++iter) {
        cout << (*iter).getName() << "="
             << (*iter).getValue() << "; ";
    }
    cout << '\n' << p() << '\n';
}

//gui stuff
void begin_form()
{
    cout << "<form name=form action=/play method=post>\n";
    cout << "<input type=hidden name='input'>\n";
    cout << "<input type=hidden name='command'>\n\n";
}
void end_form()
{
    cout << "</form>\n\n";
}
void console (string text, int height, int width)
{
    cout << "<div id='editor'>\n";
    cout << textarea().set("name","editor").set("class","input")
                      .set("cols", _2string(width))
                      .set("rows", _2string(height))
                      .set("maxlength","512") << '\n'
         << text << textarea() << '\n';
    cout << "</div>\n\n";
}
void message (string text, int height, int width)
{
    strip(text);
    if (not text.empty()) {
        height = count_lines(text, height);
        cout << "<div id='message'>\n";
        cout << textarea().set("id","output").set("class","output")
                          .set("cols", _2string(width))
                          .set("rows", _2string(height))
                          .set("readonly") << '\n'
             << text << textarea() << '\n';
        cout << "</div>\n\n";
    }
}

void alert (string message)
{
    cout << script().set("language","javascript")
                    .set("type","text/javascript") << '\n'
         << "alert('" << message << "')\n"
         << script() << '\n';
}

//grunting, a la ___
/* XXX this seems to crash
Menu::~Menu ()
{
    for (unsigned i=0; i<widgets.size(); ++i) delete widgets[i];
}
*/

Grunt& Grunt::sel_strings (string name, std::vector<string> opts)
{
    Selector* sel = new Selector(name);
    for (unsigned i=0; i<opts.size(); ++i) sel->add(opts[i]);
    add(sel);
    return *this;
}
Grunt& Grunt::sel_pow2 (string name, unsigned ub, unsigned start, string title)
{
    Selector* sel = new Selector(name,title);
    sel->sel(_2string(start));
    sel->add("0");
    for (unsigned i=1; i<=ub; i<<=1) sel->add(_2string(i));
    add(sel);
    return *this;
}

void Sep::write (std::ostream& o) const { o << " - \n"; }
void Button::write (std::ostream& o) const
{
    if (action.empty()) { o << name << '\n'; return; }
    o << "\t\t<a href=\"javascript:" << action << '"';
    if (not title.empty()) o << " title=\"" << title << '"';
    o << ">" << name << "</a>\n";
}
void Selector::write (std::ostream& o) const
{
    o << "\t\t<select name=\"" << name << '"';
    if (not title.empty()) o << " title=\"" << title << '"';
    o << ">";
    for (unsigned i=0; i<options.size(); ++i) {
        o << "\n\t\t<option";
        if (start == options[i]) o << " selected";
        o << "> " << options[i] << " </option>";
    }
    o << "\n\t\t</select>\n";
}

void Menu::menu (std::ostream& o) const
{
    o << "<td><span>" << name << "</span>\n";
    o << "\t<ul>\n";
    for (unsigned i=0; i<widgets.size(); ++i) widgets[i]->write(o);
    o << "\t</ul>\n";
    o << "</td>\n";
}
void Grunt::menu (ostream& o) const
{
    o << "<div id='menu'>\n";
    o << "<table class='center'><tr>\n";
    for (unsigned i=0; i<menus.size(); ++i) menus[i].menu(o);
    o << "</tr></table>\n";
    o << "</div>\n\n";
}
void Menu::keypad (std::ostream& o) const
{
    o << "\t<tr><td>\n";
    for (unsigned i=0; i<widgets.size(); ++i) widgets[i]->write(o);
    o << "\t</td></tr>\n";
}
void Grunt::keypad (ostream& o) const
{
    o << "<div id='keypad'>\n";
    o << "<table class='center'>\n";
    for (unsigned i=0; i<menus.size(); ++i)  menus[i].keypad(o);
    o << "</table>\n";
    o << "</div>\n\n";
}

}


