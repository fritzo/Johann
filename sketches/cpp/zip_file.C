
//see http://www.bzip.org/1.0.3/bzip2-manual-1.0.3.html#hl-interface

//============ from socket_tools.h ============

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <bzlib.h>

using namespace std;

#define EOT '\004'
#define MAX_LINE 1024
#define BUFF_SIZE 1024


class Socket
{
public:
    virtual ~Socket () {}
    virtual operator bool () const = 0;
    virtual string read (char end=EOT, unsigned max_line=MAX_LINE) = 0;
    virtual void write (string s, char end=EOT, unsigned max_line=MAX_LINE) =
0;
};

class Zipped : public Socket
{
protected:
    FILE* m_file;
    BZFILE* m_zip;
public:
    virtual operator bool () const { return m_zip; }
};

class OZipped : public Zipped
{
public:
    OZipped (string path);
    virtual ~OZipped ();

    virtual string read (char, unsigned) { return ""; }
    virtual void write (string s, char end=EOT, unsigned max_line=MAX_LINE);
};

class IZipped : public Zipped
{
    char *m_beg, *m_end;
    char m_buffer[BUFF_SIZE];
    bool get ();
public:
    IZipped (string path);
    virtual ~IZipped ();

    virtual string read (char end=EOT, unsigned max_line=MAX_LINE);
    virtual void write (string, char, unsigned) {}
};

//============ from socket_tools.C ============ 

#define CASE(num,mess) case num: cout << mess << endl; break;
inline bool bz_check (int bz_error, BZFILE* zip)
{//returns true on error
    if (bz_error == BZ_OK) return false;
    if (bz_error == BZ_STREAM_END) return true;

    cout << "bzip2 error: " << bz_error << endl;
    switch (bz_error) {

        CASE( BZ_CONFIG_ERROR,      "bzip2 library has been misconfigured" )
        CASE( BZ_PARAM_ERROR,       "parameter error" )
        CASE( BZ_IO_ERROR,          "i/o error" )
        CASE( BZ_MEM_ERROR,         "out of memory" )
        CASE( BZ_SEQUENCE_ERROR,    "mismatched read/write functions" )
        CASE( BZ_UNEXPECTED_EOF,    "unexpected EOF" )
        CASE( BZ_DATA_ERROR,        "data integrity error" )
        CASE( BZ_DATA_ERROR_MAGIC,  "data error: bad header" )
        //CASE( BZ_STREAM_END,        "end of stream detected" )

        default: cout << "unknown error" << endl;
    }
    return true;
}
#undef CASE

//writing
void bz_write_close (BZFILE** zip, FILE** file)
{
    if (!*file) return;
    int bz_error;
    BZ2_bzWriteClose(&bz_error, *zip, 0, NULL, NULL);
    bz_check(bz_error, *zip);
    if (*file) fclose(*file);
    *zip = NULL;
    *file = NULL;
}
void bz_write_open (BZFILE** zip, FILE** file, string path)
{
    *file = fopen(path.c_str(),"wb");
    if (!*file) {
        cout << "failed to open file for writing" << endl;
        zip = NULL;
        return;
    }

    int bz_error;
    *zip = BZ2_bzWriteOpen(&bz_error, *file,
            9, 0, 0); //blockSize100k=0..9, verbosity, workFactor=default
    if (bz_check(bz_error, *zip)) bz_write_close(zip, file);
}
void bz_write (BZFILE** zip, FILE** file, const char* data, int size)
{
    if (!*zip) { cout << "could not write to file" << endl; return; }
    int bz_error;
    BZ2_bzWrite(&bz_error, *zip, const_cast<char*>(data), size);
    if (bz_check(bz_error, *zip)) bz_write_close(zip, file);
}

//reading
void bz_read_close (BZFILE** zip, FILE** file)
{
    if (!*file) return;
    int bz_error;
    BZ2_bzReadClose(&bz_error, *zip);
    bz_check(bz_error, file);
    if (*file) fclose(*file);
    *zip = NULL;
    *file = NULL;
}
void bz_read_open (BZFILE** zip, FILE** file, string path)
{
    *file = fopen(path.c_str(),"rb");
    if (!*file) {
        cout << "failed to open file for writing" << endl;
        zip = NULL;
        return;
    }
    int bz_error;
    *zip = BZ2_bzReadOpen(&bz_error, *file,
            0, 0, NULL, 0); //verbosity, small, unused, unused
    if (bz_check(bz_error, *zip)) bz_read_close(zip, file);
}
int bz_read (BZFILE** zip, FILE** file, char* buffer, int buff_size)
{
    if (!*zip) { cout << "could not read from file" << endl; return 0; }
    int bz_error;
    int result = BZ2_bzRead(&bz_error, *zip, buffer, buff_size);
    if (bz_check(bz_error, file)) bz_read_close(zip, file);
    return result;
}

OZipped::OZipped (string path)  { bz_write_open (&m_zip, &m_file, path); }
OZipped::~OZipped ()            { bz_write_close(&m_zip, &m_file); }
IZipped::IZipped (string path)
    : m_beg(0), m_end(m_beg+1)  { bz_read_open (&m_zip, &m_file, path); }
IZipped::~IZipped ()            { bz_read_close(&m_zip, &m_file); }

void OZipped::write (string s, char end, unsigned max_line)
{
    if (s.size() > max_line) s = "(!) write limit exceeded";
    bz_write(&m_zip, &m_file, s.data(), s.size());
    bz_write(&m_zip, &m_file, &end, 1);
}
bool IZipped::get ()
{
    if (++m_beg < m_end) return true;

    if (m_zip) {
        m_beg = m_buffer;
        m_end = m_buffer + bz_read(&m_zip, &m_file, m_buffer, BUFF_SIZE);
        return m_beg < m_end;
    }

    m_beg = m_end;
    return false;
}
string IZipped::read (char end, unsigned max_line)
{
    string result;
    for (unsigned count=0; get() and (*m_beg != end); ++count)
    {
        if (count == max_line) return "(!) read limit exceeded";
        result.push_back(*m_beg);
    }
    return result;
}

//========== test functions ==========

void test_write (int lines=5)
{
    cout << "Testing writing:" << endl;

    cout << "--opening file" << endl;
    OZipped file("test.bz2");
    if (!file) { cout << "failed to open for reading" << endl; return; }

    cout << "--writing" << endl;
    file.write("this is a test");
    for (int i=0; i<lines; ++i) {
    file.write("this is a long line blah blah blah blah blah blah blah");
    }
    file.write("exit");
}
void test_read (int lines)
{
    cout << "Testing reading:" << endl;

    cout << "--opening file" << endl;
    IZipped file("test.bz2");
    if (!file) { cout << "failed to open for reading" << endl; return; }

    cout << "--reading" << endl;
    cout << file.read() << endl;
    for (int i=0; i<lines; ++i) file.read();
    cout << "..." << endl;
    cout << file.read() << endl;
}
int main (void)
{
    int lines = 5000;
    test_write (lines);
    test_read  (lines);

    return 0;
}

