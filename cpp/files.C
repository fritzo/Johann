
#include "files.h"
#include <cstdio>

const Version OLDEST_LOADABLE_VERSION(0,9,1,0);

namespace Files
{

//utilities
void fseek_block (FILE* file, Int block)
{
    fseek(file, blocks2bytes(block), SEEK_SET);
}
void fill_block (FILE* file)
{
    static const char emptyBlock[BLOCK_SIZE_BYTES] =
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };
    safe_fwrite(emptyBlock, BLOCK_SIZE_BYTES, 1, file);
}

//jdb files
void StructFileHeader::save_to_file (FILE* file)
{
    safe_fwrite(this, sizeof(StructFileHeader), 1, file);
}
void StructFileHeader::load_from_file (FILE* file)
{
    safe_fread(this, sizeof(StructFileHeader), 1, file);
}
void StructFileHeader::validate () const
{
    //validate versions
    if (version != VERSION) {
        logger.info() << "db was saved by other version " << version |0;
    }
    AssertV(OLDEST_LOADABLE_VERSION <= VERSION,
            "old file cannot be read by current version " << VERSION);

    //validate sizes
    const Int MAX_SIZE = 1<<16;
    AssertV(o_size < MAX_SIZE, "header.o_size is too big: " << o_size);
    AssertV(a_size < o_size*o_size, "header.a_size is too big: " << a_size);
    AssertV(c_size < o_size*o_size, "header.c_size is too big: " << c_size);
    AssertV(j_size < (o_size * (1+o_size))/2,
            "header.j_size is too big: " << j_size);

    //validate offsets
    //LATER
}

}

