
#include "sockets.h"
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/un.h>

#define SERVER  "/tmp/johann_server_socket"
#define MAXMSG  512

int main (void)
{
    int sock;
    char message[MAXMSG];
    struct sockaddr_un name;
    size_t size;
    int nbytes;

    //make the socket, then loop endlessly
    std::string filename = SERVER;
    sock = make_named_socket(filename);
    while (true) {
        //wait for a datagram
        size = sizeof (name);
        nbytes = recvfrom (sock, message, MAXMSG, 0,
                     (struct sockaddr *) & name, &size);
        if (nbytes < 0) {
            perror("recfrom (server)");
            exit(1);
        }

        //give a diagnostic message
        fprintf (stderr, "Server: got message: %s\n", message);

        //bounce the message back to the sender
        nbytes = sendto (sock, message, nbytes, 0,
                   (struct sockaddr *) & name, size);
        if (nbytes < 0) {
            perror("sendto (server)");
            exit(1);
        }
    }

    remove(filename.c_str());
    close(sock);
}
