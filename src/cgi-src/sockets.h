#ifndef JOHANN_SOCKETS_H
#define JOHANN_SOCKETS_H

#define JOHANN_PORT 64786
// = 49152 + hash("johann") % 16384 #in python

#include <string>

int make_named_socket (std::string& filename, unsigned retries=0);

#endif
