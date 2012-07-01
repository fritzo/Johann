
#include "sockets.h"
#include "definitions.h"

#include <sstream>
#include <iostream>
#include <iomanip>

#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>

#define MAX_TRIES 1000u

int make_named_socket (std::string& filename, unsigned retries)
{
    struct sockaddr_un name;
    int sock;
    size_t size;

    //create the socket 
    sock = socket(PF_LOCAL, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }
    std::cout << "created socket" << std::endl;

    //bind a name to the socket, trying a few names
    name.sun_family = AF_LOCAL;
    retries = min(retries, MAX_TRIES);
    for (int suffix=0; suffix <= retries; ++suffix) {
        std::ostringstream fname;
        fname << filename; //no suffix on first try
        if (suffix) fname << '_' << (suffix + MAX_TRIES);
        strncpy (name.sun_path, fname.str().c_str(), sizeof (name.sun_path));
        name.sun_path[sizeof(name.sun_path) - 1] = '\0';


        /* The size of the address is
         * the offset of the start of the filename,
         * plus its length,
         * plus one for the terminating null byte.
         */
        size = (offsetof (struct sockaddr_un, sun_path)
               + strlen (name.sun_path) + 1);
        if (bind(sock, (struct sockaddr *) &name, size)) {
            if (errno == EADDRINUSE) continue;
            perror("bind");
            exit(1);
        }
        filename = fname.str();
    }
    std::cout << "bound socket to address " << filename << std::endl;

    return sock;
}
