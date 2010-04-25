  // Socket library
using namespace std;
#include <iostream>
#include <string>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

class Socket {
  public:
   int sock_fd;

    Socket () {
        struct protoent *ent;

        ent = getprotobyname("tcp");

        //printf("Protocol number: %d\n", ent->p_proto);

        sock_fd = socket(AF_INET, SOCK_STREAM, ent->p_proto);

        //printf("Socket number: %d\n", sock);

        if (sock_fd == -1) {
            perror("Error");
        }
    }

    ~Socket () {
        close(sock_fd);
    }

    void connect( const char* host, int port ) {
        
    }

};

ostream& operator << (ostream& o, Socket& s) {
    o << "<socket " << s.sock_fd << ">";
    return o;
}

Socket& operator << (Socket& sock, string& s) {
    write(s.sock_fd, s.data(), s.length());
    return sock;
}

Socket& operator >> (Socket& sock, void(*)(string& s)) {

}

