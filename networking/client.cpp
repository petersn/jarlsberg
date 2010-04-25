using namespace std;
#include <iostream>
#include "sockets.cpp"

int main(int argc, char **argv) {
    Socket sock;
    cout << sock << endl;

    sock << "abc";
}

