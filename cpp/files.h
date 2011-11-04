#ifndef JOHANN_FILES_H
#define JOHANN_FILES_H

#include "definitions.h"
#include "version.h"
#include <utility>

namespace Files
{

const Logging::Logger logger("file", Logging::DEBUG);

//utilities
const Int BLOCK_SIZE = 256; //in bytes
inline Int bytes2blocks (long bytes)
{//rounds up
    return (BLOCK_SIZE - 1 + bytes) / BLOCK_SIZE;
}
inline long blocks2bytes (Int blocks)
{
    return blocks * BLOCK_SIZE;
}
void fseek_block (FILE* file, Int block);
void fill_block (FILE* file);

//jdb files
struct ObNamePair
{
    enum { MAX_NAME_SIZE = 15 };
    Int ob;
    char name[MAX_NAME_SIZE+1];
};
typedef std::pair<Int,Float> IntWMass;
struct StructFileHeader
{
    Version version;

    //integer properties
    Int props1, props2; //various properties

    //brain age
    Long age;

    //structure sizes
    Int b_size, o_size;         //number of atoms/obs
    Int a_size, c_size, j_size; //number of app/comp/join eqations
    Int w_size, r_size;         //number of words/rules in language
    Int z_size;                 //reserved for later use

    //data offsets, in units of 256B
    Int b_data; //for the atomic basis
    Int o_data; //for ob properties
    Int a_data; //for app eqns
    Int c_data; //for comp eqns
    Int j_data; //for join eqns
    Int l_data; //for the [= table
    Int L_data; //for language
    Int y_data, z_data; //reserved for later use

    //constructors
    StructFileHeader () {}
    StructFileHeader (Version v,
            Long a,
            Int bs, Int os, Int as, Int cs, Int js, Int ws, Int rs,
            Int bd, Int od, Int ad, Int cd, Int jd, Int ld, Int Ld)
        : version(v),
          props1(0), props2(0),
          age(a),
          b_size(bs), o_size(os), a_size(as), c_size(cs), j_size(js),
          w_size(ws), r_size(rs), z_size(0),
          b_data(bd), o_data(od), a_data(ad), c_data(cd), j_data(jd),
          l_data(ld), L_data(Ld), y_data(0), z_data(0)
    {}

    void save_to_file (FILE* file);
    void load_from_file (FILE* file);
    void validate () const;
};

}

#endif

