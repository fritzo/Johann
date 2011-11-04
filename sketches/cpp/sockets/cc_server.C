
#include "socket_tools.h"
using namespace SocketTools;

const char* addr = "/tmp/johann_server";

int main (void) 
{
    unixstream ustream;

    JohannSocket server;
    while (server.isPendingConnection(30000)) { //wait 30 seconds
        ustream.open(server);
        ustream << "welcome to " << addr << endl;
        if (ustream.isPending(Socket::pendingInput, 2000)) {
            int i;
            ustream >> i;
            ustream << "user entered " << i << endl;
        }
        ustream << "exiting now" << endl;
        ustream.close();
    }
}

