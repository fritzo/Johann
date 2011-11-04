
#include "sockets.h"
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SERVER  "/tmp/johann_server_socket"
#define CLIENT  "/tmp/johann_client_socket"
#define MAXMSG  512
#define MESSAGE "Yow!!! Are we having fun yet?!?"

int main (void)
{
    int sock;
    char message[MAXMSG];
    struct sockaddr_un name;
    size_t size;
    int nbytes;

    //make the socket
    std::string filename = CLIENT;
    sock = make_named_socket(filename);

    //initialize the server socket address
    name.sun_family = AF_UNIX;
    strcpy(name.sun_path, SERVER);
    size = strlen (name.sun_path) + sizeof (name.sun_family);

    //send the datagram
    nbytes = sendto (sock, MESSAGE, strlen (MESSAGE) + 1, 0,
                     (struct sockaddr *) & name, size);
    if (nbytes < 0) {
        perror("sendto (client)");
        exit(1);
    }

    //wait for a reply
    nbytes = recvfrom (sock, message, MAXMSG, 0, NULL, 0);
    if (nbytes < 0) {
        perror("recfrom (client)");
        exit(1);
    }

    //print a diagnostic message
    fprintf (stderr, "Client: got message: %s\n", message);

    //clean up
    remove(filename.c_str());
    close(sock);
}
