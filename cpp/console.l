
%x COMMENT
%x COMMAND
%x READING
%x TESTING
%x PLOTTING
%x FORGETTING
%x ABSTRACTED
%x SET_PARAM
%x PROPS
%x WEIGHT
%x USING_

%{
const int CODE = 0;   // = INITIAL
const int TEXT = 1;   // = COMMENT

/*========== C declarations ==========*/

#define YY_ALWAYS_INTERACTIVE false
#define YY_NEVER_INTERACTIVE false
#define YY_MAIN false
#define YY_STACK_USED false

#include "socket_tools.h"

//this is the bison-generated token type table
#include "console.h"
#include "console.tab.h"
#include <readline/readline.h>
#include <readline/history.h>

//state flags
bool clear_context ()
{
    bool result = true;
    result = result and contextStack.empty();   contextStack.clear();
    result = result and startStack->empty();    startStack->clear();
    //result = result and YY_START == 0;        BEGIN(CODE);
    return result;
}

// use readline with flex
#ifdef YY_INPUT
#undef YY_INPUT
#endif
#define YY_INPUT(buf, result, max_size) \
    result = readline_to_lex(buf, max_size);

//file reading stack
enum FileType { JCODE, JTEXT };
FileType get_ftype (string& name)
{
    int len = name.size();
    if (len>=6 and name.substr(len-6,len) == ".jtext") {
        return JTEXT;
    } else {
        return JCODE;
    }
}
std::set<string> g_open_files, g_read_files; //to prevent rereading
class FileInfo
{
    string m_name;
    FILE* m_file;
    FileType m_type;
    YY_BUFFER_STATE m_buffer;
    int m_line_number;
    bool m_line_was_blank;
    bool m_quiet;
    bool m_skimming;
    bool m_interactive;
    std::vector<string> m_reading_queue;
    StartStack m_start_stack;
public:
    bool  interactive       () { return m_interactive; }
    bool& line_was_blank    () { return m_line_was_blank; }
    int&  line_number       () { return m_line_number; }
    const string& name () { return m_name; }

    FileInfo () { Error( "default FileInfo constructor called" ); }

    void switch_to (bool keep_state=false)
    {
        logger.info() << "switching to file " << m_name |0;
        logger.debug() << "reading " << (m_type ? "jtext" : "jcode") |0;

        g_quiet = m_quiet;
        g_skimming = m_skimming;
        K::set_paused(not m_interactive);

        yyin = m_file;
        yy_switch_to_buffer(m_buffer);

        if (keep_state) startStack->push_back(YY_START);
        startStack = &(m_start_stack);
        logger.debug() << "BEGIN(" << m_start_stack.back() << ")" |0;
        BEGIN(m_start_stack.back());
        m_start_stack.pop_back();
    }

    FileInfo (string name, FILE* file,
              bool quiet, bool skimming, bool inter)
        : m_name(name),
          m_file(file),
          m_type(get_ftype(name)),
          m_buffer(yy_create_buffer(file, YY_BUF_SIZE)),
          m_line_number(1),
          m_line_was_blank(false),
          m_quiet(quiet),
          m_skimming(skimming),
          m_interactive(inter),
          m_reading_queue(0)
    {
        m_start_stack.push_back(CODE);
        if (m_type == JTEXT) m_start_stack.push_back(TEXT);
        switch_to (true);
    }

    ~FileInfo ()
    {
        Assert (startStack = &m_start_stack, "wrong start stack");
        switch (m_type) {
            case JTEXT:
                AssertW(m_start_stack.size() >= 1,
                        "unfinished code block at end of jtext file");
                AssertW(m_start_stack.size() <= 1,
                        "unfinished comment block at end of jtext file");
                m_start_stack.clear();
                break;
            case JCODE:
                AssertW(m_start_stack.size() == 0,
                        "unfinished comment block at end of jcode file");
        }
        if (not clear_context()) { //mods start stack
            j_error() << "unfinished context at end of file" << std::endl;
            logger.error() << "unfinished context at end of file" |0;
        }
        startStack = NULL;
        logger.info() << "finished  reading " << m_name |0;
        yy_delete_buffer(m_buffer);
        fclose(m_file);
        g_open_files.erase(m_name);
    }
};
std::vector<FileInfo*> g_files;
bool listening () { return yyin == stdin; }
bool interactive () { return g_files.back()->interactive(); }
void clear_files () { g_read_files.clear(); }
void push_input ()
{
    if (g_open_files.find("stdin") != g_open_files.end()) {
        logger.debug() << "stdin already opened; skipping" |0;
        return;
    }
    g_open_files.insert("stdin");

    g_quiet = g_quietly and g_interactive_output and (not g_interactive_input);
    K::set_paused(false);
    FileInfo *rl_file = new FileInfo("stdin", stdin, g_quiet, false, true);
    g_files.push_back(rl_file);
}
bool push_file (string filename, bool skimming, bool whine)
{
    if (g_open_files.find(filename) != g_open_files.end()) {
        logger.debug() << "file already opened; skipping" |0;
        return true;
    }

    if ((not listening())
        and (g_read_files.find(filename) != g_read_files.end())) {
        logger.debug() << "file already read; skipping" |0;
        return true;
    }

    FILE* file = fopen(filename.c_str(), "r");
    if (not file) {
        if (whine) logger.error() << "failed    opening " << filename |0;
        return false;
    }

    //open the file & start reading
    g_open_files.insert(filename);
    g_read_files.insert(filename);

    g_skimming = g_skimming or skimming; //keep skimming if already doing so
    g_quiet = g_quietly;
    K::set_paused(true);
    FileInfo *new_file = new FileInfo(filename,file, g_quiet,g_skimming, false);
    g_files.push_back(new_file);

    return true;
}
bool pop_file ()
{
    delete g_files.back();
    g_files.pop_back();
    if (g_files.empty()) {
        logger.info() << "no more files to switch to; exiting" |0;
        return false;
    } else {
        g_files.back()->switch_to();
        return true;
    }
}
void restart (bool notation)
{
    logger.debug() << "restarting console" |0;
    Logging::IndentBlock block;

    clear_context();
    clear_files();
    if (notation) push_file("scripts/notation.jtext");
}
inline bool next_line (bool blank = false)
{
    FileInfo& file = *(g_files.back());
    bool result = file.line_was_blank();
    file.line_was_blank() = blank;
    ++file.line_number();
    return result;
}
int get_line_number () { return g_files.back()->line_number(); }
string get_location ()
{
    std::ostringstream loc;
    loc << g_files.back()->name() << ':' << get_line_number();
    return loc.str();
}

//a wrapper for readline input
extern FILE *rl_instream;
int readline_to_lex (char* buf, unsigned int max_size)
{//readline has no max buffer size, but lex does
    //a static buffer for readline's results
    static string line_buffer;
    static const char *pos = 0;
    static size_t chars_left = 0;
    const char* new_line;

    //if yyin differs from readline input, just read it
    if (not listening()) {
        return fread(buf, 1, max_size, yyin);
    }

    if (chars_left == 0) {
        //read new line
        bool continued_line = not(contextStack.empty() and startStack->empty());
        if (g_quiet or continued_line) {
            //XXX: this shouldn't use readline
            new_line = readline("");
        } else {
            new_line = readline(in_prompt);
        }
        if (!new_line) new_line = "exit"; //exit on EOF
        line_buffer = new_line;
        if (g_interactive_input) {
            add_history(new_line);
        } else {
            free(const_cast<char*>(new_line)); // ? why are we freeing this ?
        }
        line_buffer.push_back('\n');
        if (not (g_quiet or g_interactive_input)) {
            std::cout << line_buffer;
        }
        pos = line_buffer.c_str();
        chars_left = line_buffer.size();
    }

    //copy part of line buffer to lex buffer
    size_t size = (max_size > chars_left ? chars_left : max_size);
    memcpy (buf, pos, size);
    pos        += size;
    chars_left -= size;

    return size;
}

inline int abrupt_EOL()
{
    BEGIN(CODE);
    clear_context();
    return EOL;
}

%}

NAME    [_a-zA-Z'][_a-zA-Z0-9']*
FNAME        [^ \t\n;]+[^ \t\n;.]
FLOAT   -?[0-9]*[.][0-9]+
NAT     [0-9]+
INT     -[0-9]+
WS      [ \t]+

%%

  /* two blank lines in a row terminate context */
<*>^\n {
        if (next_line(true) and g_interactive_input
                            and YY_START == CODE
                            and not contextStack.empty()) {
            j_error() << "unfinished context after two blank lines at "
                      << get_location() << std::endl;
            logger.error() << "unfinished context after two blank lines at "
                           << get_location() |0;
            return abrupt_EOL();
        }
        if (contextStack.empty()) return EOL;
    }

  /* comments */
<*>("###>"|"###{"|"###[") {
        //j_debug() << get_line_number() << ": begin comment" << std::endl;
        startStack->push_back(YY_START);
        if (not contextStack.empty()) {
            contextStack.clear();
            j_error() << "unfinished context at end of code block at "
                      << get_location() << std::endl;
            logger.error() << "unfinished context at end of code block at "
                           << get_location() |0;
        }
        BEGIN(COMMENT);
    }
<COMMENT>{
  ("###<"|"###}"|"###]") {
            //j_debug() << get_line_number() << ": end comment" << std::endl;
            Assert (not startStack->empty(), "extra closing bracket");
            BEGIN(startStack->back());
            startStack->pop_back();
        }
  [^#\n]* { /* ignore comments */ }
  \n { next_line(); }
}
<*>"#".* { /* ignore comments */ }

  /* reg exprs */
<*>{WS}  { /* ignore white space */ }
\n      { next_line(); if (contextStack.empty()) return EOL; }
<<EOF>> { clear_context(); if (not pop_file()) yyterminate(); }
{NAT}   { yylval.nat_val = atoi(yytext); return NAT; }
{INT}   { yylval.int_val = atoi(yytext); return INT; }

  /* help */
<*>^"help" {
        tempStrings.insert(yylval.str_val = new string(""));
        return HELP;
    }
<*>^"help "[a-zA-Z0-9_]* {
        tempStrings.insert(yylval.str_val = new string(yytext+5));
        return HELP;
    }
<*>^"?"[a-zA-Z0-9_]* {
        tempStrings.insert(yylval.str_val = new string(yytext+1));
        return HELP;
    }
<*>^"??" {
        tempStrings.insert(yylval.str_val = new string("full"));
        return HELP;
    }
<*>"license" {
        BEGIN(INITIAL);
        tempStrings.insert(yylval.str_val = new string("copying"));
        return HELP;
    }

  /* commands */
<*>("exit"|"quit") { clear_context(); if (not pop_file()) yyterminate(); }
"!" { BEGIN(COMMAND); return '!'; }
<COMMAND>{
      /* end-command */
    "." { BEGIN(INITIAL); return '.'; }
    \n  { BEGIN(INITIAL); next_line(); if (contextStack.empty()) return EOL; }

      /* special commands */
    {FLOAT} { yylval.float_val = atof(yytext); return FLOAT; }
    "log "[^\n]* {
            BEGIN(INITIAL);
            tempStrings.insert(yylval.str_val = new string(yytext+4));
            return ECHO_THIS;
        }

      /* simple commands */
    "ls basis"  { yylval.nat_val = 1; return WHO; }
    "ls context" { yylval.nat_val = 2; return WHO; }
    "ls"        { yylval.nat_val = 3; return WHO; }
    "size"      { BEGIN(INITIAL); return SIZE; }
    "stats"     { BEGIN(INITIAL); yylval.nat_val = 0; return STATS; }
    "win"       { BEGIN(INITIAL); yylval.nat_val = 1; return STATS; }
    "apps"      { BEGIN(INITIAL); yylval.nat_val = 2; return STATS; }
    "ords"      { BEGIN(INITIAL); yylval.nat_val = 3; return STATS; }
    "masses"    { BEGIN(INITIAL); yylval.nat_val = 4; return STATS; }
    "map"       { BEGIN(INITIAL); yylval.nat_val = 5; return STATS; }
    "params"    { BEGIN(INITIAL); return PARAM; }
    "validate"  { BEGIN(INITIAL); return VALIDATE; }
    "test"      { BEGIN(INITIAL); return TEST; }
    "listen"    { return LISTEN; }
    "send lang" { return SEND_LANG; }
    "send eqns" { return SEND_EQNS; }
    "send"         { return SEND; }
    "lang"|"language"   { BEGIN(INITIAL); return LANG; }
    "think in"          { BEGIN(INITIAL); pause_EOL(); return THINK_IN; }
    "think about"|"ta"    { BEGIN(INITIAL); pause_EOL(); return THINK_ABOUT; }
    "think about everything"|"ta all"    { BEGIN(INITIAL); return TA_ALL; }
    "think about context"|"ta context"   { BEGIN(INITIAL); return TA_CONTEXT; }
    "think about theory"|"ta theory"     { BEGIN(INITIAL); return TA_THEORY; }
    "think"                              { BEGIN(INITIAL); return THINK; }
    "start thinking" { BEGIN(INITIAL); yylval.nat_val = 1; return SET_THINKING; }
    "stop thinking"  { BEGIN(INITIAL); yylval.nat_val = 0; return SET_THINKING; }
    "start resting"  { BEGIN(INITIAL); yylval.nat_val = 1; return SET_RESTING; }
    "stop resting"   { BEGIN(INITIAL); yylval.nat_val = 0; return SET_RESTING; }
    "start guessing" { BEGIN(INITIAL); yylval.nat_val = 1; return SET_GUESSING; }
    "stop guessing"  { BEGIN(INITIAL); yylval.nat_val = 0; return SET_GUESSING; }
    "nap"            { BEGIN(INITIAL); yylval.nat_val = 0; return REST; }
    "rest"           { BEGIN(INITIAL); yylval.nat_val = 1; return REST; }
    "conjecture"     { BEGIN(INITIAL); return CONJECTURE; }
    "solve'"|"s'"    { BEGIN(INITIAL); yylval.nat_val = 1; return SOLVE; }
    "solve"|"s"      { BEGIN(INITIAL); yylval.nat_val = 0; return SOLVE; }
    "simplest"       { BEGIN(INITIAL); return SIMPLEST; }
    "relevant"       { BEGIN(INITIAL); return RELEVANT; }
    "sketchy"        { BEGIN(INITIAL); return SKETCHY; }
    "define"         { BEGIN(INITIAL); pause_EOL(); return DEFINE; }
    "assume"         { BEGIN(INITIAL); pause_EOL(); return ASSUME; }
    "check"          { BEGIN(INITIAL); pause_EOL(); return CHECK; }
    "assert"         { BEGIN(INITIAL); pause_EOL(); return ASSERT; }
    "recheck"        { BEGIN(INITIAL); return RECHECK; }
    "problems"       { BEGIN(INITIAL); return PROBLEMS; }
    "errors"|"errata" { BEGIN(INITIAL); return ERRORS; }
    "mark"           { BEGIN(INITIAL); return MARK; }
    "unmark"         { BEGIN(INITIAL); return UNMARK; }

      /* context management */
    "update"    { BEGIN(INITIAL); return UPDATE; }
    "context"   { BEGIN(INITIAL); return PRINT_CONTEXT; }
    "clear"     { BEGIN(INITIAL); return CLEAR_CONTEXT; }

      /* language management */
    "retract"  { BEGIN(INITIAL); return RETRACT; }
    "extend"   { BEGIN(INITIAL); return EXTEND; }
}
<COMMAND>{
      /*restarting */
    "restart" { BEGIN(INITIAL); return RESTART; }

      /* parameter setting */
    "set" { BEGIN(SET_PARAM); return SET; }
}
<SET_PARAM>{
    "size ="                        { return SET_SIZE; }
    "granularity ="                 { return SET_GRANULARITY; }
    "time scale = "|"time-scale ="  { return SET_TIME_SCALE; }
    "density ="                     { return SET_DENSITY; }
    "temp ="|"temperature ="        { return SET_TEMP; }
    "elegance ="                    { return SET_ELEGANCE; }
    "P_app ="                       { return SET_P_APP; }
    "P_comp ="                      { return SET_P_COMP; }
    "P_join ="                      { return SET_P_JOIN; }
    /*
    "small parse" { BEGIN(INITIAL); yylval.nat_val = 1; return SET_PARSING; }
    "large parse" { BEGIN(INITIAL); yylval.nat_val = 0; return SET_PARSING; }
    */
    "quietly" { BEGIN(INITIAL); yylval.nat_val = true;  return SET_QUIETLY; }
    "loudly"  { BEGIN(INITIAL); yylval.nat_val = false; return SET_QUIETLY; }

    {NAT} { yylval.nat_val = atoi(yytext); return NAT; }
    {INT} { yylval.int_val = atoi(yytext); return INT; }
    {FLOAT} { yylval.float_val = atof(yytext); return FLOAT; }
    {FNAME} {
            tempStrings.insert(yylval.str_val= new string(yytext));
            return FILENAME;
        }
    \n { next_line(); BEGIN(INITIAL); return EOL; }
}

"@" { BEGIN(WEIGHT); return '@'; }
  /* weights */
<WEIGHT>{
    {FLOAT} { BEGIN(INITIAL); yylval.float_val = atof(yytext); return FLOAT; }
    {NAT}   { BEGIN(INITIAL); yylval.nat_val   = atoi(yytext); return NAT; }
    "," { return abrupt_EOL(); }
    "}" { return abrupt_EOL(); }
    \n { next_line(); return abrupt_EOL(); }
}

  /* reading files */
<COMMAND>{
    "save"   { BEGIN(READING); return SAVE; }
    "load"   { BEGIN(READING); return LOAD; }
    "lib"    { BEGIN(READING); return LIB; }
    "read"   { BEGIN(READING); yylval.nat_val = false; return READ; }
    "import" { BEGIN(READING); yylval.nat_val = true;  return READ; }
    "write"  { BEGIN(READING); return WRITE; }
    "dump obs"  {
            BEGIN(READING);
            yylval.nat_val = Symbols::OBS_STRUCT;
            return DUMP;
        }
    "dump apps" {
            BEGIN(READING);
            yylval.nat_val = Symbols::APP_STRUCT;
            return DUMP;
        }
    "dump comps" {
            BEGIN(READING);
            yylval.nat_val = Symbols::COMP_STRUCT;
            return DUMP;
        }
    "dump joins" {
            BEGIN(READING);
            yylval.nat_val = Symbols::JOIN_STRUCT;
            return DUMP;
        }
    "dump" { BEGIN(READING); yylval.nat_val = 0; return DUMP; }
}
<READING>{
    ",;"[ \t\n]+ { /* ignore delimiters */ }
    {FNAME} {
            tempStrings.insert(yylval.str_val= new string(yytext));
            return FILENAME;
        }
    "." { BEGIN(INITIAL); return DOT; }
    \n { next_line(); return abrupt_EOL(); }
}

  /* plotting */
<COMMAND>"plot" { BEGIN(PLOTTING); }
<PLOTTING>{
    [^ \t\n;]+ {
            yylval.nat_val = atoi(yytext);
            return PLOT;
        }
    \n { next_line(); return abrupt_EOL(); }
}

  /* syntax commands */
<COMMAND>{
    {NAT} { yylval.nat_val = atoi(yytext); return NAT; }
    "normal"|"N"   { BEGIN(INITIAL); return NORMALIZE; }
    "comb"|"C"     { BEGIN(INITIAL); return COMB; }
    "pretty"|"P"   { BEGIN(INITIAL); return PRETTY; }
    "reduce"|"R"   { BEGIN(INITIAL); return REDUCE; }
    "sample"       { BEGIN(INITIAL); return SAMPLE; }
    "expand"|"E"   { BEGIN(INITIAL); return EXPAND; }
    "compress"     { BEGIN(INITIAL); return COMPRESS; }
    "express"|"X"  { BEGIN(INITIAL); return EXPRESS; }
    "simplify"|"S" { BEGIN(INITIAL); return SIMPLIFY; }
    "grok"|"G"     { BEGIN(INITIAL); return GROK; }
}

  /* binders */
"/\\"   { yylval.nat_val = Symbols::FORALL; return BINDER; }
"\\/"   { yylval.nat_val = Symbols::EXISTS; return BINDER; }
"\\"    { yylval.nat_val = Symbols::LAMBDA; return BINDER; }

  /* using */
<COMMAND>{
    "using"        { BEGIN(USING_); return USING; }
    "without"|"wo" { BEGIN(USING_); return WITHOUT; }
    "with"|"w"     { BEGIN(USING_); return WITH; }
}
<USING_>{
    {NAME} {
            tempStrings.insert(yylval.str_val = new string(yytext));
            return NAME;
        }
    "." { BEGIN(INITIAL); return DOT; }
    "," { return ','; }
    \n { next_line(); return abrupt_EOL(); }
}

  /* logical operations */
"let"   { return LET; }
"AND"   { return AND_S; }
"OR"    { return OR_S; }
"==>"   { return IMPLIES_S; }

  /* identifiers */
"???"   {
            LOCK_SYNTAX
            tempExprs.insert(yylval.expr_val = new ExprHdl(EX::build_undef()));
            UNLOCK_SYNTAX
            return ATOM;
        }
"XXX"   {
            LOCK_SYNTAX
            tempExprs.insert(yylval.expr_val = new ExprHdl(EX::build_erron()));
            UNLOCK_SYNTAX
            return ATOM;
        }
{NAME}  {
            LOCK_SYNTAX
            tempExprs.insert(yylval.expr_val
                             = new ExprHdl(EX::build_atom(yytext)));
            UNLOCK_SYNTAX
            return ATOM;
        }

  /* relations */
":="    { pause_EOL(); return DEF_EQ; }
"<::"   { yylval.reln_val = Symbols::SUBTEST;       return RELATION; }
"::>"   { yylval.reln_val = Symbols::SUPTEST;       return RELATION; }
"<:"    { yylval.reln_val = Symbols::SUBTYPE;       return RELATION; }
":>"    { yylval.reln_val = Symbols::SUPTYPE;       return RELATION; }
"[["    { yylval.reln_val = Symbols::LESS;          return RELATION; }
"]]"    { yylval.reln_val = Symbols::GREATER;       return RELATION; }
"[!="   { yylval.reln_val = Symbols::NOT_LEQ;       return RELATION; }
"=!]"   { yylval.reln_val = Symbols::NOT_GEQ;       return RELATION; }
"!=]"   { yylval.reln_val = Symbols::NOT_GEQ;       return RELATION; }
"!="    { yylval.reln_val = Symbols::NEQUAL;        return RELATION; }
"!:"    { yylval.reln_val = Symbols::NOT_OF_TYPE;   return RELATION; }
"![="   { yylval.reln_val = Symbols::NOT_LEQ;       return RELATION; }
"!<:"   { yylval.reln_val = Symbols::NOT_SUBTYPE;   return RELATION; }
"!:>"   { yylval.reln_val = Symbols::NOT_SUPTYPE;   return RELATION; }
"!<::"  { yylval.reln_val = Symbols::NOT_SUBTEST;   return RELATION; }
"!::>"  { yylval.reln_val = Symbols::NOT_SUPTEST;   return RELATION; }
"!::"   { yylval.reln_val = Symbols::FAILED;        return RELATION; }
"[="    { yylval.reln_val = Symbols::LESS_EQUAL;    return RELATION; }
"=]"    { yylval.reln_val = Symbols::GREATER_EQUAL; return RELATION; }
"="     { yylval.reln_val = Symbols::EQUAL;         return RELATION; }
"::"    { yylval.reln_val = Symbols::TESTED;        return RELATION; }
":"     { yylval.reln_val = Symbols::OF_TYPE;       return RELATION; }

  /* various brackets */
"("     { contextStack.push (Paren);    return '('; }
")"     { contextStack.pop  (Paren);    return ')'; }
"["     { contextStack.push (Square);   return '['; }
"]"     { contextStack.pop  (Square);   return ']'; }
"<"     { contextStack.push (Angle);    return '<'; }
">"     { contextStack.pop  (Angle);    return '>'; }
"{"     { contextStack.push (Squiggle); return '{'; }
"}"     { contextStack.pop  (Squiggle); return '}'; }
"\""    {
            if (contextStack.top() == Quote) {
                contextStack.pop(Quote);
                return END_QUOTE;
            } else {
                contextStack.push(Quote);
                return BEG_QUOTE;
            }
        }

  /* misc symbols */
"/"     { return '/'; }
"*"     { return '*'; }
","     { return ','; }
"->"    { return ARROW; }
"-"     { return DASH; }
"|"     { return '|'; }
"+"     { return '+'; }
";"     { return ';'; }
"?"     { resume_EOL(); return '?'; }
"."     { resume_EOL(); return '.'; }

<*>.   { //XXX: this should be .+
            j_error() << "lexing error: " << yytext << std::endl;
            clear_context();
            BEGIN(INITIAL);
        }

%%



