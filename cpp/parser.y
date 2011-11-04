
%skeleton "lalr1.cc"
%require "2.1a"
%defines
%define "parser_class_name" "Parser"
%pure-parser
%name-prefix="PP"
%debug
%error-verbose
%locations
%initial-action
{
    //initial location
    @$.begin.filename = @$.end.filename = &driver.file;
};



%{

#include "parser.h"
#define SCANNER driver.scanner

%}

// The parsing context.
%parse-param { PP::Driver& driver }
%lex-param   { yyscan_t SCANNER }

%union
{
    string*                 str_val;
    ExprHdl*                expr_val;
    VarHdl*                 var_val;
    PattHdl*                patt_val;
    SubsHdl*                subs_val;
    std::vector<ExprHdl>*   expr_list_val;
    std::vector<PattHdl>*   patt_list_val;
    Int                     nat_val;
    Binder*                 binder_val;
}

%token YYEOF 0
%token ERROR LOCAL
%token USING IMPORT
%token <nat_val> FIND
%token <nat_val> CHECK
%token <nat_val> ASSUME
%token <str_val> FILENAME
%token <nat_val> BINDER
%token L_PAREN "("
%token R_PAREN ")"
%token L_ANGLE "<"
%token R_ANGLE ">"
%token L_BRACE "["
%token R_BRACE "]"
%token <str_val> NAME
%token DASH
%token <expr_val> ATOM
%token <nat_val> NAT

%type <expr_val> flat_expr
%type <expr_val> atom_expr
%type <expr_val> app_expr
%type <expr_val> bound_expr
%type <expr_val> rand_expr
%type <expr_val> join_expr
%type <expr_val> expr
%type <subs_val> open_subs
%type <subs_val> import
%type <subs_val> using
%type <subs_val> subs
%type <subs_val> subs_dot
%type <expr_list_val> expr_list
%type <expr_list_val> expr_vect
%type <expr_list_val> expr_tup
%type <patt_val> pattern
%type <patt_val> raw_patt
%type <patt_val> atom_patt
%type <patt_list_val> patt_list
%type <binder_val> binder

/* precedence declarations */
%left  COMMA    ","
%left  LET      "let"
%left  DEF_EQ   ":="
%left  OF_TYPE  ":"
%left  TESTED   "::"
%token PAREN_TERM
%right DOT      "."
%left  SEMI     ";"
%left  JOIN     "|"
%nonassoc RAND  "+"
%left  APPLY
%right ARROW    "->"
%left  COMPOSE  "*"

/*========== grammar definition ==========*/

%{
YY_DECL;
%}

%start input
%%

input: expr     YYEOF { driver.set_expr(*$1); }
    |  subs     YYEOF { driver.set_subs(*$1); }
    |  subs_dot YYEOF { driver.set_subs(*$1); }

 /* patterns and binders */
atom_patt: ATOM {
            Var* var = (*$1)->var();
            if (var) {
                driver.buffer($$ = new PattHdl(new EX::VarPatt(var)));
            } else {
                driver.error() << "non-variable appeared in pattern"<<std::endl;
            }
        }
raw_patt: DASH { driver.buffer($$ = new PattHdl(new EX::BlankPatt())); }
    | atom_patt { $$ = $1; }
    | L_ANGLE pattern R_ANGLE {
            driver.buffer($$ = new PattHdl(new EX::VectPatt(*$2)));
        }
    | L_ANGLE patt_list R_ANGLE {
            driver.buffer($$ = new PattHdl(new EX::VectPatt(*$2)));
        }
    | L_PAREN patt_list R_PAREN {
            driver.buffer($$ = new PattHdl(EX::build_tuple(*$2)));
        }
pattern: raw_patt { $$ = $1; }
    | pattern OF_TYPE atom_expr { $$ = $1; (*$$)->type(*$3); }
    | pattern TESTED  atom_expr { $$ = $1; (*$$)->test(*$3); }

patt_list: pattern COMMA pattern {
            driver.buffer($$ = new std::vector<PattHdl>());
            $$->push_back(*$1);
            $$->push_back(*$3);
        }
    | patt_list COMMA pattern { $$ = $1; $$->push_back(*$3); }


binder: BINDER pattern {
            Symbols::BinderType binderType = Symbols::BinderType($1);
            driver.buffer($$ = new Binder(binderType, *$2));
        }
    | binder COMMA pattern { $$ = $1; $$->push(*$3); }

 /* expressions */
atom_expr: ATOM { $$ = $1; }
    | L_PAREN expr R_PAREN %prec PAREN_TERM { $$ = $2; }
    | NAT       { driver.buffer($$ = new ExprHdl(EX::build_nat(Int($1)))); }
    | expr_vect { driver.buffer($$ = new ExprHdl(EX::build_vector(*$1))); }
    | expr_tup  { driver.buffer($$ = new ExprHdl(EX::build_tuple(*$1))); }
    /* OLD
    | subs DASH atom_expr %prec DASH {
            driver.buffer($$ = new ExprHdl((*$1)->act(*$3)));
        }
    */
    | atom_expr COMPOSE atom_expr %prec COMPOSE {
            driver.buffer($$ = new ExprHdl(EX::build_comp(*$1, *$3)));
        }
    | atom_expr ARROW atom_expr {
            driver.buffer($$ = new ExprHdl(EX::build_arrow(*$1, *$3)));
        }

app_expr: atom_expr { $$ = $1; }
    | app_expr atom_expr %prec APPLY {
            driver.buffer($$ = new ExprHdl(EX::build_app(*$1, *$2)));
        }

bound_expr: binder DOT expr %prec DOT {
            Binder &binder = *$1;
            ExprHdl &term = *$3;
            driver.buffer($$ = new ExprHdl(binder.bind(term)));
            if ((*$$)->isBad()) error(@3, "bad binder");
        }
    | BINDER DOT expr %prec DOT { $$ = $3; } //empty binder does nothing

expr: flat_expr { $$ = $1; }
    | bound_expr { $$ = $1; }
    | flat_expr DOT expr %prec DOT {
            driver.buffer($$ = new ExprHdl(EX::build_app(*$1,*$3)));
        }
    | subs_dot expr %prec DOT {
            driver.buffer($$ = new ExprHdl((*$1)->act(*$2)));
        }
    /* LATER change to subs */
    | atom_patt DEF_EQ flat_expr DOT expr %prec DOT {
            driver.buffer($$ = new ExprHdl(new EX::Definition(*$1, *$3, *$5)));
        }
    /* LATER change to subs */
    | LET pattern DEF_EQ flat_expr DOT expr %prec DOT {
            driver.buffer($$ = new ExprHdl(new EX::Definition(*$2, *$4, *$6)));
        }
    | app_expr bound_expr %prec APPLY {
            driver.buffer($$ = new ExprHdl(EX::build_app(*$1,*$2)));
        }
    | join_expr JOIN bound_expr %prec JOIN {
            driver.buffer($$ = new ExprHdl(EX::build_join(*$1,*$3)));
        }
    | flat_expr SEMI bound_expr %prec SEMI {
            driver.buffer($$ = new ExprHdl(EX::build_comp(*$3,*$1)));
        }

rand_expr: app_expr { $$ = $1; }
    | app_expr RAND app_expr %prec RAND {
            driver.buffer($$ = new ExprHdl(EX::build_rand(*$1, *$3)));
        }

join_expr: rand_expr { $$ = $1; }
    | join_expr JOIN rand_expr %prec JOIN {
            driver.buffer($$ = new ExprHdl(EX::build_join(*$1, *$3)));
        }

flat_expr: join_expr { $$ = $1; }
    | flat_expr SEMI join_expr %prec SEMI {
            driver.buffer($$ = new ExprHdl(EX::build_comp(*$3, *$1)));
        }

expr_vect: L_ANGLE R_ANGLE { driver.buffer($$ = new std::vector<ExprHdl>(0)); }
    | L_ANGLE expr R_ANGLE {
            driver.buffer($$ = new std::vector<ExprHdl>());
            $$->push_back(*$2);
        }
    | L_ANGLE expr_list R_ANGLE { $$ = $2; }

expr_tup: L_PAREN expr_list R_PAREN { $$ = $2; }

expr_list: expr COMMA expr { /* list of length at least two */
            driver.buffer($$ = new std::vector<ExprHdl>());
            $$->push_back(*$1);
            $$->push_back(*$3);
        }
    | expr_list COMMA expr { $$ = $1; $$->push_back(*$3); }

 /* substitutions */
open_subs: L_BRACE { driver.buffer($$ = new SubsHdl(new Subs())); }
    | open_subs subs_dot %prec DOT {
            $$ = $1;
            (*$$)->define(*$2);
        }
    | open_subs atom_patt DEF_EQ flat_expr DOT %prec DOT {
            $$ = $1;  (*$$)->define(*$2,*$4);
        }
    | open_subs LET pattern DEF_EQ flat_expr DOT %prec DOT {
            $$ = $1;  (*$$)->define(*$3,*$5);
        }
    | open_subs LOCAL pattern DEF_EQ flat_expr DOT %prec DOT {
            $$ = $1;  (*$$)->define(*$3,*$5,true);
        }
subs: open_subs R_BRACE { $$ = $1; (*$$)->close(driver.error()); }
    | open_subs atom_patt DEF_EQ flat_expr R_BRACE %prec DOT {
            $$ = $1;  (*$$)->define(*$2,*$4);
            (*$$)->close(driver.error());
        }
    | open_subs LET pattern DEF_EQ flat_expr R_BRACE %prec DOT {
            $$ = $1;  (*$$)->define(*$3,*$5);
            (*$$)->close(driver.error());
        }
    | open_subs LOCAL pattern DEF_EQ flat_expr R_BRACE %prec DOT {
            $$ = $1;  (*$$)->define(*$3,*$5,true);
            (*$$)->close(driver.error());
        }

using: USING { driver.buffer($$ = new SubsHdl(new Subs())); }
    | using NAME { $$ = $1; (*$$)->use(*$2); }

import: IMPORT { driver.buffer($$ = new SubsHdl(new Subs())); }
    | import FILENAME { $$ = $1; (*$$)->import(*$2); }

subs_dot: subs DOT { $$ = $1; }
    | import DOT { $$ = $1; (*$$)->close(driver.error()); }
    | using DOT { $$ = $1; (*$$)->close(driver.error()); }

 /* statements */

 /* meta-commands
meta: FIND expr DOT { }
    | CHECK stmt { }
    ASSUME stmt { }
 */
%%

/*========== additional C code ==========*/

void PP::Parser::error (const PP::location& l, const string& m)
{
    driver.error() << l << ':' << m << std::endl;
}


