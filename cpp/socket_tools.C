
#include "socket_tools.h"
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <bzlib.h>

#define BACKLOG 10

const char* write_limit_error = "# write limit exceeded";
const char* read_limit_error  = "# read limit exceeded";

namespace SocketTools
{

//======== sockets ==========

//socket tools
sockaddr* SA (sockaddr_un& sa) { return reinterpret_cast<sockaddr*>(&sa); }
void mk_addr (sockaddr_un& addr, const char* path)
{
    bzero(&addr, sizeof(sockaddr_un));
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, path);
}

//wrappers
typedef const Logging::fake_ostream& Log;
#define CASE(c,mess) case c: log << mess |0; break;
int socket_ (int domain, int type, int protocol=0)
{
    int result;
    if ((result = socket(domain, type, protocol)) < 0) {
        Log log = logger.error() << "socket error: ";
        switch (errno) {
            CASE( EACCES,       "create permission denied" )
            CASE( EAFNOSUPPORT, "unsupported address family" )
            CASE( EMFILE,       "process file table full" )
            CASE( ENFILE,       "system file limit exceeded" )
            CASE( ENOMEM,       "out of memory" )
            //CASE( EPROTONOSUPPORT, "unsported protocol" )
            default:            log << "unknown errno: " << errno   |0;
        }
    }
    return result;
}
int connect_(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{
    int result;
    if ((result = connect(sockfd, serv_addr, addrlen)) < 0) {
        Log log = logger.error() << "connect error: ";
        switch (errno) {
            CASE( EACCES,       "write permission denied" )
            CASE( EPERM,        "firewall rules prevented connection?" )
            CASE( EADDRINUSE,   "address alread used" )
            CASE( EAFNOSUPPORT, "unsupported address family" )
            CASE( EAGAIN,       "no more free local ports" )
            CASE( EALREADY,     "socket already in use" )
            CASE( EBADF,        "invalid file descriptor" )
            CASE( ECONNREFUSED, "connection refused" )
            CASE( EFAULT,       "socket address outside user space" )
            CASE( EINPROGRESS,  "wait for connection to non-blocking socket" )
            CASE( EINTR,        "system interrupted" )
            CASE( EISCONN,      "socket already connected" )
            CASE( ENETUNREACH,  "network is unreachable" )
            CASE( ENOTSOCK,     "argument was not a socket" )
            CASE( ETIMEDOUT,    "timed out" )
            default:            log << "unknown errno: " << errno   |0;
        }
    }
    return result;
}
int bind_ (int socket, struct sockaddr *addr, socklen_t length)
{
    int result;
    if ((result = bind(socket, addr, length)) < 0) {
        Log log = logger.error() << "bind error: ";
        switch (errno) {
            CASE( EACCES,       "access permission denied" )
            CASE( EADDRINUSE,   "address alread used" )
            CASE( EBADF,        "invalid file descriptor" )
            CASE( EINVAL,       "socket already has address" )
            CASE( ENOTSOCK,     "argument was not a socket" )
            CASE( EAFNOSUPPORT, "unsupported address family" )
            CASE( EFAULT,       "socket address outside user space" )
            CASE( ELOOP,        "too many symbolic links" )
            CASE( ENAMETOOLONG, "address name is too long" )
            CASE( ENOENT,       "file does not exist" )
            CASE( ENOMEM,       "out of memory" )
            CASE( ENOTDIR,      "path contains non-directory" )
            CASE( EROFS,        "socket on read-only file system" )
            default:            log << "unknown errno: " << errno   |0;
        }
    }
    return result;
}
int listen_ (int socket, int backlog=BACKLOG)
{
    int result;
    if ((result = listen(socket, backlog)) < 0) {
        Log log = logger.error() << "listen error: ";
        switch (errno) {
            CASE( EADDRINUSE,   "address alread used" )
            CASE( EBADF,        "invalid file descriptor" )
            CASE( ENOTSOCK,     "argument was not a socket" )
            //CASE( EOPNOTSUP,    "socket cannot listen" )
            default:            log << "unknown errno: " << errno   |0;
        }
    }
    return result;
}
int accept_ (int socket, struct sockaddr *addr=NULL, socklen_t *addrlen=NULL)
{
    int result;
    if ((result = accept(socket, addr, addrlen)) < 0) {
        Log log = logger.error() << "accept error: ";
        switch (errno) {
            CASE( EAGAIN,       "no connections are present" )
            CASE( EBADF,        "invalid file descriptor" )
            CASE( ECONNABORTED, "connection aborted" )
            CASE( EINTR,        "system interrupted" )
            CASE( EINVAL,       "socket already has address" )
            CASE( EMFILE,       "process file table full" )
            CASE( ENFILE,       "system file limit exceeded" )
            CASE( ENOTSOCK,     "argument was not a socket" )
            CASE( EAFNOSUPPORT, "unsupported address family" )
            CASE( EFAULT,       "socket address outside user space" )
            CASE( ENOMEM,       "out of memory" )
            CASE( EPROTO,       "protocol error" )
            default:            log << "unknown errno: " << errno   |0;
        }
    }
    return result;
}
int read_ (int fd, void* buff, size_t size)
{
    int result;
    if ((result = read(fd, buff, size)) < 0) {
        Log log = logger.error() << "read error: ";
        switch (errno) {
            CASE( EAGAIN,       "no data available" )
            CASE( EBADF,        "invalid file descriptor" )
            CASE( EFAULT,       "socket address outside user space" )
            CASE( EINVAL,       "socket already has address" )
            CASE( EISDIR,       "tried to read a directory" )
            default:            log << "unknown errno: " << errno   |0;
        }
    }
    return result;
}
int write_ (int fd, const void* buff, size_t size)
{
    int result;
    if ((result = write(fd, buff, size)) < 0) {
        Log log = logger.error() << "write error: ";
        switch (errno) {
            CASE( EAGAIN,       "no data available" )
            CASE( EBADF,        "invalid file descriptor" )
            CASE( EFAULT,       "socket address outside user space" )
            CASE( EFBIG,        "tried to write too much" )
            CASE( EINTR,        "system interrupted" )
            CASE( EINVAL,       "socket already has address" )
            CASE( EIO,          "low-level i/o error" )
            CASE( ENOSPC,       "no room to write" )
            CASE( EPIPE,        "broken pipe" )
            default:            log << "unknown errno: " << errno   |0;
        }
    }
    return result;
}
#undef CASE

//socket
bool LocalSocket::connect (const char* path)
{
    if (m_connected) {
        logger.error() << "tried to connect socket twice" |0;
        return false;
    }

    if ((m_sock_fd = socket_(AF_LOCAL, SOCK_STREAM)) < 0) return false;

    sockaddr_un addr; //server address
    mk_addr(addr, path);
    if (connect_(m_sock_fd, SA(addr), sizeof(addr)) < 0) return false;

    m_connected = true;
    logger.debug() << "connected socket" |0;
    return true;
}
void LocalSocket::disconnect ()
{
    if (not m_connected) return;
    logger.debug() << "disconnecting socket" |0;
    close(m_sock_fd);
    m_connected = false;
}
int LocalSocket::_read (int fd, void* buff, size_t size)
{
    if (not m_connected) {
        logger.warning() << "tried to read from disconnected socket" |0;
        return -1;
    }
    int result = read_(fd,buff,size);
    if (result < 0) {
        logger.warning() << "disconnected socket while writing" |0;
        disconnect();
    }
    return result;
}
int LocalSocket::_write (int fd, const void* buff, size_t size)
{
    if (not m_connected) {
        logger.warning() << "tried to write to disconnected socket" |0;
        return -1;
    }
    int result = write_(fd,buff,size);
    if (result < 0) {
        logger.warning() << "disconnected socket while writing" |0;
        disconnect();
    }
    return result;
}
void LocalSocket::write (string s, char end, unsigned max_line)
{
    if (s.size() > max_line) s = write_limit_error;
    _write(m_sock_fd, s.data(), s.size());
    _write(m_sock_fd, &end, 1);
}
string LocalSocket::read (char end, unsigned max_line)
{
    string result;
    char c;
    for (unsigned count=0;
         _read(m_sock_fd, &c, 1) == 1 and c != end;
         ++count)
    {
        if (count == max_line) return read_limit_error;
        result.push_back(c);
    }
    return result;
}

//======== client/server pair ==========

//client
Client::Client (string path)
{
    logger.debug() << "connecting client to " << path |0;
    Logging::IndentBlock block;

    connect(path.c_str());
}

//server
Server::Server (string path)
    : m_path(path), m_serving(false)
{
    logger.info() << "initializing server at " << m_path |0;

    unlink(path.c_str()); //delete if exists
    mk_addr(m_addr, path.c_str());
}
Server::~Server ()
{
    unlink(m_path.c_str()); //delete path
}

bool Server::serve (void* (*f)(void*))
{
    logger.info() << "Starting server" |0;
    Logging::IndentBlock block;

    if ((m_listen_fd = socket_ (AF_LOCAL, SOCK_STREAM)) < 0) return false;
    if (bind_(m_listen_fd, SA(m_addr), SUN_LEN(&m_addr)) < 0) return false;
    if (listen_(m_listen_fd) < 0) return false;
    chmod(m_path.c_str(), 0666); //allow anyone to read/write

    for (m_serving(true); m_serving();) {
        int talk_fd;
        if ((talk_fd = accept_(m_listen_fd)) < 0) {
            if (errno == EINTR) continue;
            else { close(m_listen_fd); return false; }
        }
        logger.debug() << "connected to client" |0;

        //start input thread
        ThreadTools::new_thread(f, new LocalSocket(talk_fd));
    }

    logger.info() << "quitting server" |0;
    close(m_listen_fd);
    return true;
}

//======== files as sockets ==========

class OutFile : public Socket
{
    ofstream m_file;
public:
    OutFile (string path) : m_file(path.c_str()) {}
    virtual ~OutFile () {}

    virtual operator bool () const { return m_file; }
    virtual string read (char, unsigned) { return ""; }
    virtual void write (string s, char end=EOT, unsigned max_line=MAX_LINE);
};
void OutFile::write (string s, char end, unsigned max_line)
{
    if (s.size() > max_line) s = write_limit_error;
    m_file << s << end;
}
Socket* new_ofile (string path) { return new OutFile(path); }

class InFile : public Socket
{
    ifstream m_file;
public:
    InFile (string path) : m_file(path.c_str()) {}
    virtual ~InFile () {}

    virtual operator bool () const { return m_file; }
    virtual string read (char end=EOT, unsigned max_line=MAX_LINE);
    virtual void write (string, char, unsigned) {}
};
string InFile::read (char end, unsigned max_line)
{
    string result;
    char c;
    for (unsigned count=0; m_file.get(c) and c!=end; ++count)
    {
        if (count == max_line) return read_limit_error;
        result.push_back(c);
    }
    return result;
}
Socket* new_ifile (string path) { return new InFile(path); }

//======== compressed files as sockets ==========

//bzip2 wrappers
#define CASE(num,mess) case num: logger.error() << mess |0; break;
inline bool bz_check (int bz_error, BZFILE* zip)
{//returns true on error
    if (bz_error == BZ_OK) return false;
    if (bz_error == BZ_STREAM_END) return true;

    logger.error() << "bzip2 error: " << bz_error |0;
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

        default: logger.error() << "unknown error" |0;
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
        logger.error() << "failed to open file for writing" |0;
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
    if (!*zip) { logger.error() << "could not write to file" |0; return; }
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
        logger.error() << "failed to open file for writing" |0;
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
    if (!*zip) { logger.error() << "could not read from file" |0; return 0; }
    int bz_error;
    int result = BZ2_bzRead(&bz_error, *zip, buffer, buff_size);
    if (bz_check(bz_error, file)) bz_read_close(zip, file);
    return result;
}

//abstract zip class
class Zipped : public Socket
{
protected:
    FILE* m_file;
    BZFILE* m_zip;
public:
    virtual operator bool () const { return m_zip; }
};

//zip writer
class OZipped : public Zipped
{
public:
    OZipped (string path) { bz_write_open (&m_zip, &m_file, path); }
    virtual ~OZipped ()   { bz_write_close(&m_zip, &m_file); }

    virtual string read (char, unsigned) { return ""; }
    virtual void write (string s, char end=EOT, unsigned max_line=MAX_LINE);
};
void OZipped::write (string s, char end, unsigned max_line)
{
    if (s.size() > max_line) s = write_limit_error;
    bz_write(&m_zip, &m_file, s.data(), s.size());
    bz_write(&m_zip, &m_file, &end, 1);
}
Socket* new_ozip (string path) { return new OZipped(path); }

//zip reader
#define BUFF_SIZE 1024
class IZipped : public Zipped
{
    char *m_beg, *m_end;
    char m_buffer[BUFF_SIZE];
    bool get ();
public:
    IZipped (string path)
        : m_beg(0),
        m_end(m_beg+1)  { bz_read_open (&m_zip, &m_file, path); }
    virtual ~IZipped () { bz_read_close(&m_zip, &m_file); }

    virtual string read (char end=EOT, unsigned max_line=MAX_LINE);
    virtual void write (string, char, unsigned) {}
};
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
        if (count == max_line) return read_limit_error;
        result.push_back(*m_beg);
    }
    return result;
}
Socket* new_izip (string path) { return new IZipped(path); }

}

