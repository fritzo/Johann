#include "order.h"
#include "obs.h"

//log levels
#define LOG_DEBUG1(mess)
#define LOG_INDENT_DEBUG1
//#define LOG_DEBUG1(mess) {logger.debug() << message |0;}
//#define LOG_INDENT_DEBUG1 Logging::IndentBlock block;

namespace Order
{

namespace O = Obs;

//temporaty sets
bool g_temp_sets_intialized = false;
Set* g_temp_sets[NUM_TEMP_SETS];
void init_temp_sets (Int size)
{
    Assert (not g_temp_sets_intialized, "tried to init temp sets twice");
    g_temp_sets_intialized = true;
    for (int n=0; n<NUM_TEMP_SETS; ++n) {
        Assert (g_temp_sets[n] = new(std::nothrow) Set(size),
                "failed to allocate temp sets");

    }
}
void clear_temp_sets ()
{
    Assert (g_temp_sets_intialized, "tried to clear temp sets twice");
    g_temp_sets_intialized = false;
    for (int n=0; n<NUM_TEMP_SETS; ++n) {
        delete g_temp_sets[n];
        g_temp_sets[n] = NULL;
    }
}

//data structures
OrdTable *g_pos_table(NULL);
OrdTable *g_neg_table(NULL);


void Ord::init (Int num_obs, bool is_full)
{
    logger.debug() << "Intializing order table" |0;
    Logging::IndentBlock block;

    Assert (g_pos_table == NULL, "tried to init g_pos_table twice");
    Assert (g_neg_table == NULL, "tried to init g_neg_table twice");

    g_pos_table = new OrdTable(num_obs, is_full);
    g_neg_table = new OrdTable(num_obs, is_full);

    init_temp_sets(num_obs);
}
void Ord::resize (Int num_obs, const Int* new2old)
{
    logger.debug() << "Resizing order table" |0;
    Logging::IndentBlock block;

    Assert (g_pos_table != NULL, "tried to resize uninitialized g_pos_table");
    Assert (g_neg_table != NULL, "tried to resize uninitialized g_neg_table");

    OrdTable *temp;

    //resize pos table
    temp = new OrdTable(num_obs);
    temp->move_from(*g_pos_table, new2old);
    delete g_pos_table;
    g_pos_table = temp;

    //resize neg table
    temp = new OrdTable(num_obs);
    temp->move_from(*g_neg_table, new2old);
    delete g_neg_table;
    g_neg_table = temp;

    //resize temp sets
    clear_temp_sets();
    init_temp_sets(num_obs);
}
void Ord::clear ()
{
    logger.debug() << "Clearing order table" |0;
    Logging::IndentBlock block;

    Assert (g_pos_table != NULL, "tried to clear g_pos_table twice");
    Assert (g_neg_table != NULL, "tried to clear g_neg_table twice");

    delete g_pos_table; g_pos_table = NULL;
    delete g_neg_table; g_neg_table = NULL;

    clear_temp_sets();
}

//validation
void validate (Int level)
{
    if (level < 1) return;
    logger.debug() << "Validating [= ordering" |0;
    Logging::IndentBlock block;

    //validate supports
    logger.debug() << "validating supports" |0;
    for (Ob::iterator iter=Ob::begin(); iter!=Ob::end(); ++iter) {
        Ob ob = *iter;
        if (ob.isUsed()) {
            Assert (g_pos_table->supports(ob),
                    "invalid: pos table does not support ob " << ob);
            Assert (g_neg_table->supports(ob),
                    "invalid: neg table does not support ob " << ob);
        } else {
            Assert (not g_pos_table->supports(ob),
                    "invalid: pos table supports absent ob " << ob);
            Assert (not g_neg_table->supports(ob),
                    "invalid: neg table supports absent ob " << ob);
        }
    }

    if (level >= 2) {
        g_pos_table->validate();
        g_neg_table->validate();
        g_pos_table->validate_disjoint(*g_neg_table);
    }
}
void test ()
{
    //DEBUG
    //g_pos_table->print_table(Ob::numUsed());
    //g_neg_table->print_table(Ob::numUsed());
}

//saving/loading
Int data_size ()
{
    return g_pos_table->data_size() + g_neg_table->data_size();
}
void save_to_file (FILE* file)
{
    g_pos_table->write_to_file(file);
    g_neg_table->write_to_file(file);
}
void load_from_file (FILE* file)
{
    g_pos_table->read_from_file(file);
    g_neg_table->read_from_file(file);
}

} // namespace Order
