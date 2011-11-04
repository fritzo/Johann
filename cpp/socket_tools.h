#ifndef JOHANN_SOCKET_TOOLS_H
#define JOHANN_SOCKET_TOOLS_H

#include "definitions.h"
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include "thread_tools.h"

#define DEFAULT_PATH "/tmp/default_socket"
#define JOHANN_PORT 64786
// = 49152 + hash("johann") % 16384 #in python

#define MAX_LINE 4000
#define EOT '\004'

namespace SocketTools
{

const Logging::Logger logger("socket", Logging::DEBUG);

//======== sockets ==========

class Socket
{
public:
    virtual ~Socket () {}
    virtual operator bool () const = 0;
    virtual string read (char end=EOT, unsigned max_line=MAX_LINE) = 0;
    virtual void write (string s, char end=EOT, unsigned max_line=MAX_LINE) = 0;
};

class LocalSocket : public Socket
{
    int m_sock_fd;
    bool m_connected;
protected:
    bool connect (const char* path); //true on success
    void disconnect ();
    int _read (int fd, void* buff, size_t size);
    int _write (int fd, const void* buff, size_t size);

public:
    LocalSocket () : m_connected(false) {}
    LocalSocket (int fd) : m_sock_fd(fd), m_connected(true) {}
    virtual ~LocalSocket () { disconnect(); }

    virtual operator bool () const { return m_connected; }
    virtual string read (char end=EOT, unsigned max_line=MAX_LINE);
    virtual void write (string s, char end=EOT, unsigned max_line=MAX_LINE);
};

//======== client/server pair ==========

class Client : public LocalSocket
{
public:
    Client (string path=DEFAULT_PATH);
    virtual ~Client () {}
};

/* only one server is intended to be run at a time.
 * however, multiple serve() threads may run concurrently.
 */
class Server
{
    string m_path;
    int m_listen_fd;
    ThreadTools::ThreadSafe<bool> m_serving;
    sockaddr_un m_addr;

public:
    Server (string path=DEFAULT_PATH);
    virtual ~Server ();

    bool serve (void* (*f)(void*)); //f(&talk_fd)'s output is ignored

    //users can stop serving
    void stop_serving () { m_serving(false); }
};

//======== various socket factories ==========

//text files
Socket* new_ofile (string path);
Socket* new_ifile (string path);

//bzip2 files
Socket* new_ozip (string path);
Socket* new_izip (string path);


}

#endif
