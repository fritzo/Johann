
%{
/*========== C declarations ==========*/

#include "console.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <readline/readline.h>

extern int yylex(void);

Buffer<string>           tempStrings;
Buffer<std::vector<string> > tempStringLists;
Buffer<ExprHdl>               tempExprs;
Buffer<VarHdl>                tempVars;
Buffer<std::vector<ExprHdl> > tempExprLists;
Buffer<ExprPMF>               tempExprPMFs;
Buffer<std::vector<Int> >     tempIntLists;
Buffer<PattHdl>               tempPatts;
Buffer<std::vector<PattHdl> > tempPattLists;
Buffer<Binder>                tempBinders;
Buffer<StmtHdl>               tempStmts;

int yyerror (const char *s)
{
    logger.error() << "parse error at " << get_location() << ": " << s |0;
    if (listening()) {
        j_error() << "parse error: " << s << std::endl;
    } else {
        j_error() << "parse error at " << get_location() << ": "
                  << s << std::endl;
    }
    return 0;
}

void problem_error (bool failed, StmtHdl stmt)
{
    const char* message = failed ? "failed assertion" : "unverified assertion";

    const Logging::fake_ostream& os = failed ? logger.error() : logger.warning();
    os << message << " at " << get_location() << ":\n\t"
       << stmt |0;

    ostream& out = failed ? j_error() : j_warning();
    if (listening()) {
        out << message
            << ":\n\t" << stmt << std::endl;
    } else {
        out << message << " at " << get_location() << ":\n\t"
            << stmt << std::endl;
    }
}

void print_help_message (string filename)
{
    if (filename.empty()) filename = "brief";

    string helpfile = "scripts/help/" + filename + ".jcode";
    ifstream helpFile(helpfile.c_str());
    if (helpFile) {
        //jcode version
        ostream& os = j_info();
        char buffer[256];
        while (!helpFile.eof()) {
            //highlight until # symbol
            helpFile.getline(buffer,256);
            os << "\e[1m";
            for (unsigned i=0; buffer[i]; ++i) {
                if (buffer[i] == '#') {
                    os << (i ? "\e[0m" : "\e[32m"); //green or dull
                }
                os << buffer[i];
            }
            os << "\e[0m";
            os << std::endl;
        }
        helpFile.close();
    } else {
        helpfile = "scripts/help/" + filename + ".text";
        helpFile.open(helpfile.c_str());
        if (helpFile) {
            //text version
            ostream& os = j_info();
            char buffer[256];
            while (!helpFile.eof()) {
                helpFile.getline(buffer,256);
                os << buffer << std::endl;
            }
            helpFile.close();
        } else {
            j_warning() << "unknown help file: " << filename << std::endl;
            return;
        }
    }
}

void validate_system (Int level=3)
{
    ostream& os = j_info ()
        << "validating (level " << level << ")... " << std::flush;
    if (K::validate(level)) {
        os << "structure is valid";
    } else {
        os << "repaired invalid structure; see log file";
    }
    os << std::endl;
}

/*========== saving/loading ==========*/
string default_name("default");
void save_system (string name)
{
    default_name = name;

    K::set_paused(true);

    //save structure
    std::ostringstream db_name;
    db_name << "data/" << name << ".jdb";
    if (not K::save(db_name.str())) {
        j_error(1) << "failed to save to db " << db_name.str() << std::endl;
    }

    //save context and params
    std::ostringstream codename;
    codename << "data/" << name << ".jcode";
    ofstream file(codename.str().c_str());
    if (not file) {
        j_error(1) << "failed to save context and params " << name << std::endl;
    } else {
        logger.info() << "Saving context and params to " << codename.str() |0;
        file << "#\\title{ Saved System }\n";
        file << "#\\author{ Johann " << VERSION << " }\n";
        file << "#\\date{ " << get_date() << " }\n";
        file << "#\\maketitle\n\n";

        //LOCK_SYNTAX
        file << "#\\section{Language}\n\n";
        K::save_basis_to(file);
        context().save_to(file);
        K::save_theory_to(file);
        K::save_params_to(file);
        //UNLOCK_SYNTAX

        file.close();
    }

    //clean up
    K::set_paused(not interactive());
}
void load_system (std::vector<string>* names=NULL)
{
    std::vector<string> default_names;
    if (names == NULL) {
        default_names.push_back(default_name);
        names = &default_names;
    }

    K::set_paused(true);

    //load structure
    bool db_found = false;
    string name;
    for (Int i=0; i<names->size(); ++i) {
        name = names->operator[](i);
        std::ostringstream db_name;
        db_name << "data/" << name << ".jdb";
        if (K::load(db_name.str())) {
            db_found = true;
            default_name = name;
            break;
        }
        j_warning(1) << "failed to load to db " << db_name.str()
                   << std::endl;
    }

    //load context and params
    if (not db_found) {
        j_error(1) << "no db loaded" << std::endl;
    } else {
        restart(false);
        std::ostringstream codename;
        codename << "data/" << name << ".jcode";
        if (not push_file(codename.str(), true, false)) {
            j_error(1) << "failed to load context and params " << codename.str()
                << std::endl;
            restart();
        }
    }
}

/*========== context interaction ==========*/

ExprHdl purify (ExprHdl expr)
{
    //expand; reduce
    expr = K::expand(expr);
    const Expr::VarSet& vars = expr->vars();

    //warn about & remove any free variables
    if (vars) {
        j_error(1) << "undefined variables at " << get_location()
                   << ": " << vars << std::endl;
        logger.error() << "undefined variables at " << get_location()
                       << ": " << vars |0;
    }

    return expr = expr->as_pure();
}
std::vector<ExprHdl> purify (std::vector<ExprHdl>& exprs)
{
    std::vector<ExprHdl> result;
    for (Int i=0; i<exprs.size(); ++i) {
        result.push_back(purify(exprs[i]));
    }
    return result;
}
ExprPMF purify (ExprPMF& exprs, bool simplify = true)
{
    ExprPMF result;
    for (Int i=0; i<exprs.size(); ++i) {
        result.push_back(std::make_pair(purify(exprs[i].first),
                                        exprs[i].second));
    }
    return result;
}
void define (PattHdl patt, ExprHdl meaning)
{
    //check pattern
    patt = patt->as_comb();
    if (patt->isBad()) {
        j_error(1) << "bad pattern at " << get_location() << ": "
                   << patt << std::endl;
        logger.error() << "bad pattern at " << get_location() << ": "
                       << patt |0;
        return;
    }

    //check for errors and undefinedness
    meaning = meaning->as_comb();
    if (meaning->isBad()) {
        j_error(1) << "bad meaning at " << get_location() << ": "
                   << meaning << std::endl;
        logger.error() << "bad meaning at " << get_location() << ": "
                       << meaning |0;
        return;
    }

    //complain about any free variables
    const Expr::VarSet& vars = context().expand(meaning)->vars();
    if (vars) {
        j_error(1) << "undefined variables at " << get_location()
                   << ": " << vars << std::endl;
        logger.error() << "undefined variables at " << get_location()
                       << ": " << vars |0;
        return;
    }

    //define in unexpanded form
    if (not context().define(patt,meaning)) {
        j_error(1) << "bad definition at " << get_location() << ": "
                   << patt << " := ..." << std::endl;
        logger.error() << "bad definition at " << get_location() << ": "
                       << patt |0;
    }
}

%}

/*========== Bison declarations ==========*/

%union
{
    unsigned char           char_val;
    string*                 str_val;
    std::vector<string>*    str_list_val;
    ExprHdl*                expr_val;
    VarHdl*                 var_val;
    std::vector<ExprHdl>*   expr_list_val;
    ExprPMF*                expr_pmf_val;
    Float                   float_val;
    Int                     nat_val;
    int                     int_val;
    std::vector<Int>*       int_list_val;
    PattHdl*                patt_val;
    std::vector<PattHdl>*   patt_list_val;
    Binder*                 binder_val;
    Symbols::Relation       reln_val;
    Int                     prop_val;
    StmtHdl*                stmt_val;
    std::vector<StmtHdl>*   stmt_list_val;
}

%token EOL
%token LISTEN PARAM SET LANG VALIDATE TEST
%token THINK
%token SEND SEND_LANG SEND_EQNS
%token <nat_val> REST
%token <nat_val> SET_THINKING
%token <nat_val> SET_RESTING
%token THINK_IN THINK_AT THINK_ABOUT TA_ALL TA_CONTEXT TA_THEORY
%token RETRACT EXTEND
%token RESTART SAVE LOAD
%token NORMALIZE COMB PRETTY REDUCE SAMPLE EXPAND
%token COMPRESS SIMPLIFY GROK EXPRESS SIZE
%token SIMPLEST RELEVANT SKETCHY CONJECTURE
%token <nat_val> SOLVE
%token <nat_val> SET_GUESSING
%token DEFINE ASSUME CHECK ASSERT RECHECK PROBLEMS ERRORS MARK UNMARK
%token PRINT_CONTEXT UPDATE WRITE CLEAR_CONTEXT
%token USING WITH WITHOUT LIB
%token <nat_val> READ
%token <nat_val> DUMP
%token <nat_val> WHO
%token <nat_val> STATS
%token <nat_val> PLOT
%type <str_list_val> using with without
%type <str_list_val> filenames
%token <str_val> HELP
%token <str_val> ECHO_THIS

%token SET_P_APP SET_P_COMP SET_P_JOIN
%token SET_SIZE SET_GRANULARITY SET_TIME_SCALE SET_DENSITY SET_TEMP SET_ELEGANCE
%token <nat_val> SET_QUIETLY

%token <nat_val> BINDER
%token '(' ')' '[' ']' '{' '}' '<' '>' BEG_QUOTE END_QUOTE
%token <str_val> FILENAME
%token <str_val> NAME
%token DASH
%token <expr_val> ATOM
%token <reln_val> RELATION

%token <nat_val> NAT
%token <int_val> INT
%token <float_val> FLOAT
%type <int_val> integer
%type <float_val> number

%type <expr_val> expr
%type <expr_val> pure_expr
%type <expr_val> flat_expr
%type <expr_val> atom_expr
%type <expr_val> meta_expr
%type <expr_val> app_expr
%type <expr_val> arrow_expr
%type <expr_val> rand_expr
%type <expr_val> join_expr
%type <expr_val> semi_expr
%type <expr_val> bound_expr
%type <expr_val> app_bound_expr
%type <expr_val> arrow_bound_expr
%type <expr_val> rand_bound_expr
%type <expr_val> join_bound_expr
%type <expr_val> semi_bound_expr
%type <expr_list_val> expr_list
%type <expr_list_val> expr_vect
%type <expr_list_val> expr_tup
%type <expr_list_val> expr_set
%type <expr_pmf_val> expr_pmf
%type <patt_val> raw_patt
%type <patt_val> atom_patt
%type <patt_val> pattern
%type <patt_list_val> patt_list
%type <binder_val> binder

%type <stmt_val> reln_stmt
%type <stmt_val> bound_stmt
%type <stmt_val> flat_stmt
%type <stmt_val> statement

/* precedence declarations */
%left OR_S
%left AND_S
%right IMPLIES_S
%left LET DEF_EQ
%token PAREN_TERM
%right '.' DOT
%left  ';' SEMI
%right  '|' JOIN
%nonassoc '+' RAND
%right ARROW
%left  APPLY
%right  '*' COMPOSE

%type <nat_val> input
%start input

%%

/*========== grammar definition ==========*/

/* general commands */
input: /* empty */ { $$=true; }
    | input EOL { $$ = $1; }
    | input '!' EOL { $$=true; }
    | input HELP EOL { print_help_message(*$2); clear_buffers(); $$=true; }
    | input '!' ECHO_THIS EOL {
            if ($1) {
                user_logger.info() << *$3 |0;
                j_info() << '#' << *$3 << std::endl;
                clear_buffers();
            }
            $$=true;
        }
    | input '!' SET SET_QUIETLY EOL { set_quietly($4); $$=true; }
    | input '!' LISTEN EOL { push_input(); $$=true; }
    | input '!' READ filenames DOT EOL {
            bool skimming = $3;
            std::vector<string> &files = *$4;
            for (int i=files.size()-1; i>=0; --i) { //push backwards
                std::ostringstream jtext, jcode;
                jtext << "scripts/" << files[i] << ".jtext";
                jcode << "scripts/" << files[i] << ".jcode";
                //try jtext first; jcode otherwise
                if (not push_file(jtext.str(), skimming, false)) {
                if (not push_file(jcode.str(), skimming, false)) {
                  j_error(1) << "failed to open file " << files[i] << std::endl;
                }}
            }
            clear_buffers();
            $$=true;
        }
    | input '!' LIB filenames DOT EOL {
            std::vector<string> &files = *$4;
            for (unsigned i=0; i<files.size(); ++i) {
                std::ostringstream err;
                LOCK_SYNTAX
                S::library.add_file(files[i], err);
                UNLOCK_SYNTAX
                std::string err_str = err.str();
                if (not err_str.empty()) {
                    j_error(1) << "failed to lib " << files[i] << std::endl;
                    j_error(1) << err_str << std::flush;
                }
            }
            clear_buffers();
            $$=true;
        }
    | input '!' FILENAME EOL {
            std::ostringstream jcode;
            jcode << "command/" << *$3 << ".jcode";
            //try jtext first; jcode otherwise
            if (not push_file(jcode.str(), false, false)) {
                j_error(1) << "command not found: " << *$3 << std::endl;
            }
            clear_buffers();
            $$=true;
        }
    | input '!' VALIDATE     EOL { validate_system(); $$=true; }
    | input '!' VALIDATE NAT EOL { validate_system($4); $$=true; }
/* communication tools */
    | input '!' SEND_LANG EOL { K::send_lang(); $$=true; }
    | input '!' SEND_EQNS EOL { K::send_eqns(); $$=true; }
    | input '!' SEND EOL { K::send_lang(); K::send_eqns(); $$=true; }
/* expr tools */
    | input flat_expr '.' {
            LOCK_SYNTAX
            j_info() << *$2 << "." << std::endl;
            clear_buffers();
            UNLOCK_SYNTAX
            $$=true;
        }
    | input '!' SIZE flat_expr '.' EOL {
            LOCK_SYNTAX
            Float result = K::get_symbols(*$4);
            clear_buffers();
            if (std::isnan(result)) { j_info() << "???"  << std::endl; }
            else                    { j_info() << result << std::endl; }
            UNLOCK_SYNTAX
            $$=true;
        }
    | input '!' WHO EOL {
            LOCK_SYNTAX
            if ($3 & 1) {
                ostream& os = j_info() << "constants: ";
                EX::write_consts_to(os);
                os << std::endl;
            }
            if ($3 & 2) {
                j_info() << "definitions: " << context() << std::endl;
            }
            UNLOCK_SYNTAX
            $$=true;
        }
    | input '!' SIMPLEST     EOL { K::print_simplest_to(j_info()); $$=true; }
    | input '!' SIMPLEST NAT EOL { K::print_simplest_to(j_info(),$4); $$=true; }
    | input '!' RELEVANT     EOL { K::print_most_relevant_to(j_info()); $$=true; }
    | input '!' RELEVANT NAT EOL { K::print_most_relevant_to(j_info(),$4); $$=true; }
    | input '!' SKETCHY      EOL { K::print_sketchiest_to(j_info()); $$=true; }
    | input '!' SKETCHY NAT  EOL { K::print_sketchiest_to(j_info(),$4); $$=true; }
    | input '!' THINK_ABOUT expr_set '.' EOL {
            LOCK_SYNTAX
            K::think_about_exprs(*$4);
            clear_buffers();
            UNLOCK_SYNTAX
            $$=true;
        }
/* basis selection */
    //XXX with and without still allow parsing of bad terms...
    | input '!' with DOT    { $$ = $1 and K::with_atoms    (*$3); }
    | input '!' without DOT { $$ = $1 and K::without_atoms (*$3); }
    | input '!' using DOT {
            if ($1) {
                K::start_using(*$3);
                clear_buffers();
            }
            $$=true;
        }
    | input '!' DEFINE ATOM DEF_EQ pure_expr '.' {
            LOCK_SYNTAX
            if (not $1) clear_buffers(); else {
            string name = *((*$4)->name());
            ExprHdl meaning = *$6;
            clear_buffers(); //so variable atoms can become constant atoms
            if (not K::name_expr(name, meaning, get_location())) {
                j_error(1) << "can't name expression at " << get_location()
                           << ": " << meaning << std::endl;
                logger.error() << "can't name expression at " << get_location()
                               << ": " << meaning |0;
            } else {
                j_info() << name << " =: " << meaning << "." << std::endl;
            }
            meaning.clear();
            }
            UNLOCK_SYNTAX
            $$=true;
        }
/* structure tools */
    | input '!' PARAM EOL { K::write_params_to(j_info()); $$=true; }
    | input '!' SET EOL { K::write_params_to(j_info()); $$=true; }
    | input '!' SET SET_P_APP FLOAT EOL { K::set_P_app($5); $$=true; }
    | input '!' SET SET_P_COMP FLOAT EOL { K::set_P_comp($5); $$=true; }
    | input '!' SET SET_P_JOIN FLOAT EOL { K::set_P_join($5); $$=true; }
    | input '!' SET SET_SIZE NAT EOL { K::set_size($5); $$=true; }
    | input '!' SET SET_GRANULARITY NAT EOL { K::set_granularity($5); $$=true; }
    | input '!' SET SET_TIME_SCALE NAT EOL { K::set_time_scale($5); $$=true; }
    | input '!' SET SET_DENSITY number EOL { K::set_density($5); $$=true; }
    | input '!' SET SET_TEMP number EOL { K::set_temperature($5); $$=true; }
    | input '!' SET SET_ELEGANCE number EOL { K::set_elegance($5); $$=true; }
    | input '!' TEST EOL {
            ostream& os = j_info () << "testing... " << std::flush;
            if (K::test()) os << "tests succeeded";
            else            os << "tests failed; see log file";
            os << std::endl;
            $$=true;
        }
    | input '!' TA_ALL EOL { K::think_about_everything(); $$=true; }
    | input '!' TA_CONTEXT EOL { K::think_about_context(); $$=true; }
    | input '!' TA_THEORY EOL { K::think_about_theory(); $$=true; }
    | input '!' THINK EOL {
            if (listening()) K::think_user();
            else             K::think_auto();
            $$=true;
        }
    | input '!' NAT THINK EOL {
            Int num_cycles = $3;
            if (listening()) K::think_user(num_cycles);
            else             K::think_auto(num_cycles);
            $$=true;
        }
    | input '!' SET_THINKING EOL { K::set_thinking($3); $$=true; }
    | input '!' SET_RESTING EOL { K::set_resting($3); $$=true; }
    | input '!' REST EOL {
            switch ($3) {
                case 0: K::nap(); break;
                case 1: K::rest(); break;
            }
            $$=true;
        }
/* measure tools */
    | input '!' SET_GUESSING EOL { K::set_guessing($3); $$=true; }
    | input '!' STATS EOL {
            if ($3 == 0) {
                K::write_stats_to(j_info());
            } else if (K::save_stats(Kernel::StatType($3))) {
                j_error() << "failed to save statistics" << std::endl;
            }
            $$=true;
        }
    | input '!' PLOT EOL {
            switch ($3) {
            case 0: K::plot_funs_of_eps(); break;
            default:
                j_error(1) << "plot #" << $3
                    << " is not defined" << std::endl;
            }
            $$=true;
        }
    | input '!' NAT PLOT EOL {
            switch ($4) {
            case 0: K::plot_funs_of_eps($3); break;
            default:
                j_error(1) << "plot #" << $4
                    << " is not defined" << std::endl;
            }
            $$=true;
        }
/* lambda-theory tools */
    | input flat_stmt '.' {
            LOCK_SYNTAX
            j_info() << *$2 << "." << std::endl;
            clear_buffers();
            UNLOCK_SYNTAX
            $$=true;
        }
    | input '!' NORMALIZE flat_stmt '.' {
            LOCK_SYNTAX
            StmtHdl stmt = *$4;
            stmt = stmt->query_nf();
            j_info() << stmt << "." << std::endl;
            stmt.clear();
            clear_buffers();
            UNLOCK_SYNTAX
            $$=true;
        }
    | input flat_stmt '?' {
            LOCK_SYNTAX
            if (StmtHdl stmt = K::expand(*$2)) {
                switch (K::query_stmt(stmt)) {
                case Symbols::TRUE:    j_info() << "true."  << std::endl; break;
                case Symbols::FALSE:   j_info() << "false." << std::endl; break;
                case Symbols::UNKNOWN:
                    Symbols::Prob prob = K::guess_stmt(stmt);
                    j_info() << "???. #evidence = " << prob << std::endl;
                    break;
                }
            } else {
                j_error(1) << "bad statement syntax at "
                           << get_location() << std::endl;
                logger.error() << "bad statement syntax at "
                               << get_location() |0;
            }
            clear_buffers();
            UNLOCK_SYNTAX
            $$=true;
        }
    | input '!' CHECK flat_stmt '.' {
            LOCK_SYNTAX
            if ($1) {
            if (StmtHdl stmt = K::expand(*$4)) {
                switch (K::check_stmt(stmt, get_location(), not g_skimming)) {
                case Symbols::TRUE: break;
                case Symbols::FALSE:   problem_error(true,  *$4); break;
                case Symbols::UNKNOWN: problem_error(false, *$4); break;
                }
            } else {
                j_error(1) << "bad statement syntax at "
                           << get_location() << std::endl;
                logger.error() << "bad statement syntax at "
                               << get_location() |0;
            }
            }
            clear_buffers();
            UNLOCK_SYNTAX
            $$=true;
        }
    | input '!' ASSERT flat_stmt '.' {
            LOCK_SYNTAX
            if ($1) {
            if (StmtHdl stmt = K::expand(*$4)) {
                switch (K::query_stmt(stmt)) {
                case Symbols::TRUE: break;
                case Symbols::FALSE:   problem_error(true,  *$4); break;
                case Symbols::UNKNOWN: problem_error(false, *$4); break;
                }
            } else {
                j_error(1) << "bad statement syntax at "
                           << get_location() << std::endl;
                logger.error() << "bad statement syntax at "
                               << get_location() |0;
            }
            }
            clear_buffers();
            UNLOCK_SYNTAX
            $$=true;
        }
    | input '!' ASSUME flat_stmt '.' {
            LOCK_SYNTAX
            if ($1) {
            if (StmtHdl stmt = K::expand(*$4)) {
                if (not K::assume_stmt(stmt, get_location())) {
                    j_error() << "assumption failed at "
                              << get_location() << std::endl;
                    logger.error() << "assumption failed at "
                                   << get_location() |0;
                }
            } else {
                j_error(1) << "bad statement syntax at "
                           << get_location() << std::endl;
                logger.error() << "bad statement syntax at "
                               << get_location() |0;
            }
            }
            clear_buffers();
            UNLOCK_SYNTAX
            $$=true;
        }
    | input '!' RECHECK EOL { K::review(); K::recheck(); $$=true; }
    | input '!' PROBLEMS EOL {
            ostream& os = j_info();
            K::problems(os,Symbols::UNKNOWN);
            os << std::flush;
            $$=true;
        }
    | input '!' ERRORS EOL {
            //write to console
            ostream& os = j_info();
            K::problems(os,Symbols::FALSE);
            os << std::flush;

            //save to file
            string name = "scripts/errata.jcode";
            ofstream file(name.c_str());
            if (!file) {
                logger.warning() << "unable to save to " << name |0;
            } else {
                logger.info() << "saving errors to " << name |0;
                file << "#\\title{ Errata }\n";
                file << "#\\author{ Johann " << VERSION << " }\n";
                file << "#\\date{ " << get_date() << " }\n";
                file << "#\\maketitle\n\n";
                K::problems(file,Symbols::FALSE);
                file.close();
            }

            $$=true;
        }
    | input '!' SOLVE flat_stmt '.' EOL {
            K::solve_and_print(j_info(), *$4, $3);
            $$=true;
        }
    | input '!' CONJECTURE EOL {
            K::print_conjectures_to(j_info());
            $$=true;
        }
    | input '!' CONJECTURE NAT EOL {
            K::print_conjectures_to(j_info(), $4);
            $$=true;
        }
    | input '!' MARK   EOL { K::mark_all(); $$=true; }
    | input '!' UNMARK EOL { K::unmark_all(); $$=true; }
    | input '!' MARK flat_expr '.' EOL {
            LOCK_SYNTAX
            K::mark(*$4);
            clear_buffers();
            UNLOCK_SYNTAX
            $$=true;
        }
    | input '!' UNMARK flat_expr '.' EOL {
            LOCK_SYNTAX
            K::unmark(*$4);
            clear_buffers();
            UNLOCK_SYNTAX
            $$=true;
        }
/* context tools */
    /* TODO: try to avoid recompiling a context every time sth is defined */
    | input atom_patt DEF_EQ flat_expr '.' {
            LOCK_SYNTAX
            if ($1) define(*$2,*$4);
            clear_buffers();
            UNLOCK_SYNTAX
            K::recompile_context();
            $$=true;
        }
    | input LET raw_patt DEF_EQ flat_expr '.' {
            LOCK_SYNTAX
            if ($1) define(*$3,*$5);
            clear_buffers();
            UNLOCK_SYNTAX
            K::recompile_context();
            $$=true;
        }
    | input '!' UPDATE EOL { K::update(); $$=true; }
    | input '!' PRINT_CONTEXT EOL {
            LOCK_SYNTAX
            context().write_to(j_info());
            UNLOCK_SYNTAX
            $$=true;
        }
    | input '!' WRITE FILENAME EOL {
            std::ostringstream filename;
            filename << "scripts/user/" << *$4 << ".jcode";
            clear_buffers();
            ofstream file(filename.str().c_str());
            if (not file) {
                j_error(1) << "failed to open file " << *$4 << std::endl;
            } else {
                file << "#\\title{Saved Context}\n";
                file << "#\\author{Johann " << VERSION << "}\n";
                file << "#\\date{ " << get_date() << "}\n";
                file << "#\\maketitle\n\n";

                LOCK_SYNTAX
                context().write_to(file);
                //context().save_to(file);
                //K::save_theory_to(file);
                UNLOCK_SYNTAX

                file.close();
            }
            $$=true;
        }
    | input '!' CLEAR_CONTEXT EOL {
            LOCK_SYNTAX
            context().clear();
            K::clear_theory();
            UNLOCK_SYNTAX
            $$=true;
        }
/* language development tools */
    | input '!' LANG EOL { K::write_lang_to(j_info()); $$=true; }
    | input '!' THINK_IN expr_set '.' EOL {
            LOCK_SYNTAX
            unsigned size = $4->size();
            if (size < 2) {
                j_warning() << "thinking in small basis: size = "
                            << size << std::endl;
            }
            if (not K::think_in(purify(*$4))) {
                j_error() << "I don't know all those words" << std::endl;
            }
            clear_buffers();
            UNLOCK_SYNTAX
            $$=true;
        }
    | input '!' THINK_IN expr_pmf '.' EOL {
            LOCK_SYNTAX
            unsigned size = $4->size();
            if (size < 2) {
                j_warning() << "thinking in small basis: size = "
                            << size << std::endl;
            }
            if (not K::think_in(purify(*$4))) {
                j_error() << "I don't know all those words" << std::endl;
            }
            clear_buffers();
            UNLOCK_SYNTAX
            $$=true;
        }
    | input '!' RETRACT pure_expr EOL {
            LOCK_SYNTAX
            if (not K::retract(*$4)) {
                j_warning() << "retraction failed" << std::endl;
            }
            clear_buffers();
            UNLOCK_SYNTAX
            $$=true;
        }
    | input '!' EXTEND pure_expr EOL {
            LOCK_SYNTAX
            if (not K::extend(*$4)) {
                j_warning() << "extension failed" << std::endl;
            }
            clear_buffers();
            UNLOCK_SYNTAX
            $$=true;
        }
/* saving, loading, etc. */
    | input '!' RESTART EOL { K::restart(); restart(); $$=true; }
    | input '!' DUMP FILENAME EOL {
            std::ostringstream filename;
            filename << "data/" << *$4;
            K::dump(filename.str(), $3);
            clear_buffers();
            $$=true;
        }
    | input '!' SAVE FILENAME EOL {
            save_system(*$4);
            clear_buffers();
            $$=true;
        }
    | input '!' SAVE EOL {
            save_system(default_name);
            clear_buffers();
            $$=true;
        }
    | input '!' LOAD filenames EOL {
            load_system($4);
            clear_buffers();
            $$=true;
        }
    | input '!' LOAD EOL {
            load_system();
            clear_buffers();
            $$=true;
        }
/* errors */
    | input error EOL {
            LOCK_SYNTAX
            clear_buffers();
            UNLOCK_SYNTAX
            yyerrok;
            $$=true;
        }

with: WITH NAME {
            //logger.info() << "with..." |0;
            tempStringLists.insert($$ = new std::vector<string>);
            $$->push_back(*$2);
        }
    | with NAME { $$ = $1; $$->push_back(*$2); }
without: WITHOUT NAME {
            //logger.info() << "without..." |0;
            tempStringLists.insert($$ = new std::vector<string>);
            $$->push_back(*$2);
        }
    | without NAME { $$ = $1; $$->push_back(*$2); }
using: USING NAME {
            tempStringLists.insert($$ = new std::vector<string>);
            $$->push_back(*$2);
        }
    | using NAME { $$ = $1; $$->push_back(*$2); }
    | using ',' NAME { $$ = $1; $$->push_back(*$3); } /* for backwards compat */

filenames: FILENAME {
            tempStringLists.insert($$ = new std::vector<string>);
            $$->push_back(*$1);
        }
    | filenames FILENAME { $$ = $1; $$->push_back(*$2); }

/*======================== patterns and binders ========================*/

atom_patt: ATOM {
            LOCK_SYNTAX
            Var* var = (*$1)->var();
            if (var) {
                tempPatts.insert($$ = new PattHdl(new EX::VarPatt(var)));
            } else {
                j_error() << "non-variable appeared in pattern at "
                          << get_location() << ": " << *$1 << std::endl;
                logger.error() << "non-variable appeared in pattern at "
                               << get_location() << ": " << *$1 |0;
                tempPatts.insert($$ = new PattHdl(new EX::BlankPatt()));
            }
            UNLOCK_SYNTAX
        }

raw_patt: DASH {
            LOCK_SYNTAX
            tempPatts.insert($$ = new PattHdl(new EX::BlankPatt()));
            UNLOCK_SYNTAX
        }
    | atom_patt { $$ = $1; }
    /* unnecessary:
    | '<' '>' {
            LOCK_SYNTAX
            tempPatts.insert($$ = new PattHdl(new EX::VectPatt()));
            UNLOCK_SYNTAX
        }
    */
    | '<' pattern '>' {
            LOCK_SYNTAX
            tempPatts.insert($$ = new PattHdl(new EX::VectPatt(*$2)));
            UNLOCK_SYNTAX
        }
    | '<' patt_list '>' {
            LOCK_SYNTAX
            tempPatts.insert($$ = new PattHdl(new EX::VectPatt(*$2)));
            UNLOCK_SYNTAX
        }
    | '(' patt_list ')' {
            LOCK_SYNTAX
            tempPatts.insert($$ = new PattHdl(EX::build_tuple(*$2)));
            UNLOCK_SYNTAX
        }

pattern: raw_patt { $$ = $1; }
    | pattern RELATION flat_expr {
            LOCK_SYNTAX
            $$ = $1;
            switch ($2) {
                case Symbols::OF_TYPE:  (*$$)->type(*$3); break;
                case Symbols::TESTED:   (*$$)->test(*$3); break;
                default:
                j_error() << "ignoring bad pattern relation at "
                          << get_location() << ": "
                          << Symbols::RelationNames[$2] << std::endl;
                logger.error() << "ignoring bad pattern relation at "
                               << get_location() << ": "
                               << Symbols::RelationNames[$2] |0;
            }
            UNLOCK_SYNTAX
        }

patt_list: pattern ',' pattern {
            tempPattLists.insert($$ = new std::vector<PattHdl>());
            $$->push_back(*$1);
            $$->push_back(*$3);
        }
    | patt_list ',' pattern { $$ = $1; $$->push_back(*$3); }

binder: BINDER pattern {
            LOCK_SYNTAX
            Symbols::BinderType binderType = Symbols::BinderType($1);
            tempBinders.insert($$ = new Binder(binderType, *$2));
            UNLOCK_SYNTAX
        }
    | binder ',' pattern {
            LOCK_SYNTAX
            $$ = $1;
            $$->push(*$3);
            UNLOCK_SYNTAX
        }

/*======================== statement syntax ========================*/

reln_stmt: flat_expr RELATION flat_expr {
            LOCK_SYNTAX
            tempStmts.insert($$ = new StmtHdl(
                ST::build_reln(*$1, Symbols::Relation($2), *$3)));
            UNLOCK_SYNTAX
        }
    | reln_stmt RELATION flat_expr {
            LOCK_SYNTAX
            ExprHdl lhs = (*$1)->rhs();
            StmtHdl reln = ST::build_reln(lhs, Symbols::Relation($2), *$3);
            tempStmts.insert($$ = new StmtHdl(ST::And(*$1, reln)));
            reln.clear();
            lhs.clear();
            UNLOCK_SYNTAX
        }

flat_stmt: reln_stmt { $$ = $1; }
    | flat_stmt AND_S flat_stmt {
            LOCK_SYNTAX
            tempStmts.insert($$ = new StmtHdl(ST::And(*$1, *$3)));
            UNLOCK_SYNTAX
        }
    | flat_stmt OR_S flat_stmt {
            LOCK_SYNTAX
            tempStmts.insert($$ = new StmtHdl(ST::Or(*$1, *$3)));
            UNLOCK_SYNTAX
        }
    | flat_stmt IMPLIES_S flat_stmt {
            LOCK_SYNTAX
            tempStmts.insert($$ = new StmtHdl(ST::Implies(*$1, *$3)));
            UNLOCK_SYNTAX
        }
    | '(' bound_stmt ')' { $$ = $2; }
    | '(' expr_list RELATION expr ')' {
            LOCK_SYNTAX
            std::vector<ExprHdl> &lhs = *$2;
            Symbols::Relation rel = $3;
            ExprHdl &rhs = *$4;
            ST::Conjunction *conj = new ST::Conjunction();
            tempStmts.insert($$ = new StmtHdl(conj));
            for (unsigned i=0; i<lhs.size(); ++i) {
                conj->add(ST::build_reln(lhs[i], rel, rhs));
            }
            UNLOCK_SYNTAX
        }

bound_stmt: binder '.' statement %prec DOT {
            LOCK_SYNTAX
            tempStmts.insert($$ = new StmtHdl($1->bind(*$3)));
            UNLOCK_SYNTAX
        }
    | atom_patt DEF_EQ flat_expr '.' statement %prec DOT {
            LOCK_SYNTAX
            tempStmts.insert($$ = new StmtHdl(new ST::Definition(*$1,*$3,*$5)));
            UNLOCK_SYNTAX
        }
    | LET raw_patt DEF_EQ flat_expr '.' statement %prec DOT {
            LOCK_SYNTAX
            tempStmts.insert($$ = new StmtHdl(new ST::Definition(*$2,*$4,*$6)));
            UNLOCK_SYNTAX
        }

statement: flat_stmt { $$ = $1; }
    | bound_stmt { $$ = $1; }

/*======================== combinator syntax ========================*/

atom_expr: ATOM { $$ = $1; }
    | '(' expr ')' %prec PAREN_TERM { $$ = $2; }
    | '[' expr ']' %prec PAREN_TERM {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(K::simplify(*$2,INFINITY)));
            UNLOCK_SYNTAX
        }
    | BEG_QUOTE statement END_QUOTE {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl((*$2)->to_bool()));
            UNLOCK_SYNTAX
         }
    | NAT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_nat(Int($1))));
            UNLOCK_SYNTAX
        }
    | expr_vect {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_vector(*$1)));
            UNLOCK_SYNTAX
        }
    | expr_tup {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_tuple(*$1)));
            UNLOCK_SYNTAX
        }
    | atom_expr '*' atom_expr %prec COMPOSE {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_comp(*$1, *$3)));
            UNLOCK_SYNTAX
        }

app_expr: atom_expr { $$ = $1; }
    | app_expr atom_expr %prec APPLY {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_app(*$1, *$2)));
            UNLOCK_SYNTAX
        }

meta_expr: '!' COMB flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl((*$3)->as_comb()));
            UNLOCK_SYNTAX
        }
    | '!' PRETTY flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl((*$3)->pretty()));
            UNLOCK_SYNTAX
        }
    | '!' NAT PRETTY flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl((*$4)->pretty($2)));
            UNLOCK_SYNTAX
        }
    | '!' REDUCE flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl((*$3)->reduce()));
            UNLOCK_SYNTAX
        }
    | '!' SAMPLE flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl((*$3)->sample()));
            UNLOCK_SYNTAX
        }
    | '!' NAT REDUCE flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl((*$4)->reduce($2)));
            UNLOCK_SYNTAX
        }
    | '!' NAT SAMPLE flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl((*$4)->sample($2)));
            UNLOCK_SYNTAX
        }
    | '!' EXPAND flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(K::expand(*$3)));
            UNLOCK_SYNTAX
        }
    | '!' integer EXPAND flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(K::expand(*$4, $2)));
            UNLOCK_SYNTAX
        }
    | '!' COMPRESS flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(K::compress(*$3)));
            UNLOCK_SYNTAX
        }
    | '!' integer COMPRESS flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(K::compress(*$4, $2)));
            UNLOCK_SYNTAX
        }
    | '!' SIMPLIFY flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(K::simplify(*$3)));
            UNLOCK_SYNTAX
        }
    | '!' EXPRESS flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(K::express(*$3)));
            UNLOCK_SYNTAX
        }
    | '!' number SIMPLIFY flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(K::simplify(*$4,$2)));
            UNLOCK_SYNTAX
        }
    | '!' GROK flat_expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(K::simplify(*$3,INFINITY)));
            UNLOCK_SYNTAX
        }

 /* XXX need to differentiate between flat_join_expr and join_expr
  * eg see scripts/skj/simple.jtext line 161, in the definition of Simple.
  */

bound_expr: binder '.' expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl($1->bind(*$3)));
            UNLOCK_SYNTAX
        }
    | BINDER '.' expr %prec DOT { $$ = $3; } //empty binder does nothing

app_bound_expr: bound_expr { $$ = $1; }
    | app_expr bound_expr %prec APPLY {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_app(*$1,*$2)));
            UNLOCK_SYNTAX
        }

arrow_bound_expr: app_bound_expr { $$ = $1; }
    | app_expr ARROW arrow_bound_expr %prec ARROW {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_arrow(*$1, *$3)));
            UNLOCK_SYNTAX
        }

rand_bound_expr: arrow_bound_expr { $$ = $1; }
    | arrow_expr '+' arrow_bound_expr %prec RAND {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_rand(*$1,*$3)));
            UNLOCK_SYNTAX
        }

join_bound_expr: rand_bound_expr { $$ = $1; }
    | join_expr '|' rand_bound_expr %prec JOIN {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_join(*$1,*$3)));
            UNLOCK_SYNTAX
        }

semi_bound_expr: join_bound_expr { $$ = $1; }
    | semi_expr ';' join_bound_expr %prec SEMI {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_comp(*$3,*$1)));
            UNLOCK_SYNTAX
        }

expr: flat_expr { $$ = $1; }
    | semi_bound_expr { $$ = $1; }
    | flat_expr '.' expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_app(*$1,*$3)));
            UNLOCK_SYNTAX
        }
    | atom_patt DEF_EQ flat_expr '.' expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(new EX::Definition(*$1,*$3,*$5)));
            UNLOCK_SYNTAX
        }
    | LET raw_patt DEF_EQ flat_expr '.' expr %prec DOT {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(new EX::Definition(*$2,*$4,*$6)));
            UNLOCK_SYNTAX
        }

arrow_expr: app_expr { $$ = $1; }
    | app_expr ARROW arrow_expr %prec ARROW {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_arrow(*$1, *$3)));
            UNLOCK_SYNTAX
        }

rand_expr: arrow_expr { $$ = $1; }
    | arrow_expr '+' arrow_expr %prec RAND {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_rand(*$1, *$3)));
            UNLOCK_SYNTAX
        }

join_expr: rand_expr { $$ = $1; }
    | join_expr '|' rand_expr %prec JOIN {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_join(*$1, *$3)));
            UNLOCK_SYNTAX
        }

semi_expr: join_expr { $$ = $1; }
    | semi_expr ';' join_expr %prec SEMI {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(EX::build_comp(*$3, *$1)));
            UNLOCK_SYNTAX
        }

flat_expr: semi_expr { $$ = $1; }
    | meta_expr { $$ = $1; }

pure_expr: flat_expr {
            LOCK_SYNTAX
            tempExprs.insert($$ = new ExprHdl(purify(*$1)));
            UNLOCK_SYNTAX
        }

expr_vect: '<' '>' { tempExprLists.insert($$ = new std::vector<ExprHdl>(0)); }
    | '<' expr '>' {
            tempExprLists.insert($$ = new std::vector<ExprHdl>());
            LOCK_SYNTAX
            $$->push_back(*$2);
            UNLOCK_SYNTAX
        }
    | '<' expr_list '>' { $$ = $2; }
    | '<' expr_list ',' '>' { $$ = $2; }

expr_tup: '(' expr_list ')' { $$ = $2; }
    | '(' expr_list ',' ')' { $$ = $2; }

expr_list: expr ',' expr { /* list of length at least two */
            tempExprLists.insert($$ = new std::vector<ExprHdl>());
            LOCK_SYNTAX
            $$->push_back(*$1);
            $$->push_back(*$3);
            UNLOCK_SYNTAX
        }
    | expr_list ',' expr {
            $$ = $1;
            LOCK_SYNTAX
            $$->push_back(*$3);
            UNLOCK_SYNTAX
        }

integer: NAT { $$ = $1; } | INT { $$ = $1; }
number: FLOAT { $$ = $1; } | NAT { $$ = $1; } | INT { $$ = $1; }

expr_set: flat_expr {
            tempExprLists.insert($$ = new std::vector<ExprHdl>());
            LOCK_SYNTAX
            $$->push_back(*$1);
            UNLOCK_SYNTAX
        }
    |  expr_set ',' flat_expr {
            $$ = $1;
            LOCK_SYNTAX
            $$->push_back(*$3);
            UNLOCK_SYNTAX
        }

expr_pmf: expr_set '@' number {
            LOCK_SYNTAX
            tempExprPMFs.insert($$ = new ExprPMF);
            Float weight = $3;

            std::vector<ExprHdl>& head = *$1;
            for (Int i=0; i<head.size(); ++i) {
                $$->push_back(std::make_pair(head[i],weight));
            }
            UNLOCK_SYNTAX
        }
    | expr_pmf ',' expr_set '@' number {
            LOCK_SYNTAX
            $$ = $1;
            Float weight = $5;

            std::vector<ExprHdl>& head = *$3;
            for (Int i=0; i<head.size(); ++i) {
                $$->push_back(std::make_pair(head[i],weight));
            }
            UNLOCK_SYNTAX

        }

%%

/*========== additional C code ==========*/

const int CANCEL_TASK_KEY = 0x1B; //escape key
int cancel_task (int,int)
{
    j_info() << "cancelling tasks" << std::endl;
    K::cancel_task();
    return 0;
}

extern FILE *rl_instream;
void* start_parser (void* _init_script)
{
    const char* init_script = static_cast<const char*>(_init_script);
    g_interactive_input = is_input_interactive();
    g_interactive_output = is_output_interactive();
    Assert (g_interactive_output or (not g_interactive_input),
            "console started with interactive input and non-interactive output");

    //set xterm title
    if (g_interactive_output) {
        std::cout << "\033]0;Johann Kernel " << VERSION << "\007";
    }

    //bind task-cancellation to some key
    //LATER: input replace with rl_bind_keyseq or rl_generic_bind
    rl_bind_key (CANCEL_TASK_KEY, cancel_task);

    //start parsing prompt
    if (g_interactive_output && g_interactive_input) {
        std::cout << "\e[1mJohann Kernel " << VERSION << "\e[0m"
                  //<< " (" << __DATE__ << ")"
                  << " copyright (c) 2004-2009 Fritz Obermeyer" << std::endl;
    }
    if (init_script) push_file(init_script);
    else             push_input();
    push_file("scripts/notation.jtext");
    yyparse();

    //clear xterm title
    if (g_interactive_output) {
        std::cout << "\033]0;\007";
    }

    K::no_more_cmds();

    pthread_exit(NULL);
    return NULL;
}



