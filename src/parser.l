
%option reentrant
%option prefix="PP"
%option yylineno
 /* %option bison-bridge */
 /* %option bison-locations */

%x COMMENT
%x FILES
%x NAMES

%{

/*========== C declarations ==========*/

#include "parser.h"
#include "parser.tab.h"

#define YY_ALWAYS_INTERACTIVE false
#define YY_NEVER_INTERACTIVE true
#define YY_MAIN false
#define YY_STACK_USED false

#define YY_EXTRA_TYPE PP::Driver*
#define DRIVER (*(yyextra))

//see C++ section 10.2.4 of bison manual
//#undef yywrap
//#define yywrap() 1

//get yyterminate to return a token
#define yyterminate() return token::YYEOF

// use readline with flex
#define YY_INPUT(buf, result, max_size) \
    { result = DRIVER.input().readsome(buf, max_size); }

//report errors to driver
void yyerror (PP::Driver& driver, const char* message)
{
    driver.error() << message << '\n';
}

using namespace PP;
typedef Parser::token token;

%}

%option noyywrap nounput batch

NAME    [_a-zA-Z'][_a-zA-Z0-9']*
FNAME   [_a-zA-Z0-9/]+
NAT     [0-9]+
WS      [ \t\n\r]+

%%

  /* comments */
<*>"###>" { BEGIN(COMMENT); }
<COMMENT>{
    "###<" { BEGIN(INITIAL); }
    [^#]* { /* ignore comments */ }
}
<*>"#".* { /* ignore comments */ }

  /* reg exprs */
<*>{WS}  { /* ignore white space */ }
<<EOF>> { yyterminate(); }
{NAT}   { yylval->nat_val = atoi(yytext); return token::NAT; }

  /* commands */
"!find"     { yylval->nat_val = yylineno; return token::FIND; }
"!check"    { yylval->nat_val = yylineno; return token::CHECK; }
"!assume"   { yylval->nat_val = yylineno; return token::ASSUME; }
"!using"    { BEGIN(NAMES); return token::USING; }
"!import"   { BEGIN(FILES); return token::IMPORT; }

<NAMES>{
{NAME} {
            DRIVER.buffer(yylval->str_val = new string(yytext));
            return token::NAME;
        }
"," { /* ignore delimiters */ }
"." { BEGIN(INITIAL); return token::DOT; }
}
<FILES>{
{FNAME} {
            DRIVER.buffer(yylval->str_val = new string(yytext));
            return token::FILENAME;
        }
"," { /* ignore delimiters */ }
"." { BEGIN(INITIAL); return token::DOT; }
}

"let" { return token::LET; }

  /* identifiers */
"XXX"   { DRIVER.buffer(yylval->expr_val = new ExprHdl(EX::bad())); return token::ATOM; }
"???"   { DRIVER.buffer(yylval->expr_val = new ExprHdl(EX::build_undef()));      return token::ATOM; }
"local" { return token::LOCAL; }
{NAME}  { DRIVER.buffer(yylval->expr_val = new ExprHdl(EX::build_atom(yytext))); return token::ATOM; }

  /* binders */
"/\\"   { yylval->nat_val = Symbols::FORALL; return token::BINDER; }
"\\/"   { yylval->nat_val = Symbols::EXISTS; return token::BINDER; }
"\\"    { yylval->nat_val = Symbols::LAMBDA; return token::BINDER; }

  /* relations */
":="    { return token::DEF_EQ; }
"::"    { return token::TESTED; }
":"     { return token::OF_TYPE; }

 /* various brackets */
"("     { DRIVER.push_context (Paren);    return token::L_PAREN; }
"<"     { DRIVER.push_context (Angle);    return token::L_ANGLE; }
"["     { DRIVER.push_context (Brace);    return token::L_BRACE; }
")"     { return DRIVER.pop_context(Paren) ? token::R_PAREN : token::ERROR; }
">"     { return DRIVER.pop_context(Angle) ? token::R_ANGLE : token::ERROR; }
"]"     { return DRIVER.pop_context(Brace) ? token::R_BRACE : token::ERROR; }

  /* misc symbols */
"*"     { return token::COMPOSE; }
","     { return token::COMMA; }
"->"    { return token::ARROW; }
"-"     { return token::DASH; }
"|"     { return token::JOIN; }
"+"     { return token::RAND; }
";"     { return token::SEMI; }
"."     { return token::DOT; }

<*>.   { //XXX: this should be .+
            DRIVER.error() << "lexing error: " << yytext << std::endl;
            return token::ERROR;
        }

%%

namespace PP
{

Driver::Driver (string fname)
    : m_input(NULL), m_error(NULL), file(fname)
{
    yylex_init(&scanner);
    yyset_extra(this, scanner);
}
Driver::~Driver ()
{
    yylex_destroy(scanner);
}

Driver::ResultType Driver::parse (istream& in, ostream& err, FileType ftype)
{
    m_input = &in;
    m_error = &err;
    yyrestart(NULL, scanner);

    //XXX this is untested
    yyguts_t* yyg = static_cast<yyguts_t*>(scanner);
    BEGIN(ftype); //a macro using yyg

    result_expr.clear();
    result_subs.clear();
    Parser parser(*this);
    parser.parse();
    clear_buffers();

    ResultType type = ERROR;
    if (result_expr) type = EXPR;
    if (result_subs) type = SUBS;
    //if (not type)           err << "parse error: no result\n";
    if (not end_context())  err << "parse error: unfinished context\n";
    err << std::flush;

    m_input = NULL;
    m_error = NULL;

    return type;
}

}


