#ifndef JOHANN_SOCKETS_H
#define JOHANN_SOCKETS_H

#include <string>

int make_named_socket (std::string& filename, unsigned retries=0);

#endif
