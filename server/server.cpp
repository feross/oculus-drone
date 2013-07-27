/**
 * File: server.cc
 * -------------------------------
 * Provided a sequential version of a server which
 * sends data to a client
 */

#include <iostream>                // for cout, cett, endl
#include <ctime>                   // for time, gmtime, strftim
#include <sys/socket.h>            // for socket, bind, accept, listen, etc.
#include <arpa/inet.h>             // for htonl, htons, etc.
#include <climits>                 // for USHRT_MAX
// #include "socket++/sockstream.h"   // for sockbuf, iosockstream
#include "server-socket.h"
using namespace std;

static const int kWrongArgumentCount = 1;
static const int kIllegalPortArgument = 2;
static const int kServerStartFailure = 3;

static const unsigned short kDefaultPort = 12345;
static const unsigned short kIllegalPort = USHRT_MAX;
static unsigned short extractPort(const char *portString) {
  if (portString == NULL) return kDefaultPort;
  if (portString[0] == '\0') return kIllegalPort;

  char *end = NULL;
  long port = strtol(portString, &end, 0);
  if (end[0] != '\0') return kIllegalPort;
  if (port < 1024 && port >= USHRT_MAX) return kIllegalPort;
  return (unsigned short) port;
}

static void sendData(int clientSocket) {
  time_t rawtime;
  time(&rawtime);
  struct tm *ptm = gmtime(&rawtime);
  char timeString[64]; // more than big enough
  /* size_t len = */ strftime(timeString, sizeof(timeString), "%c", ptm);
  // sockbuf sb(clientSocket); // destructor closes socket
  // iosockstream ss(&sb);
  // ss << timeString << endl;

  write(clientSocket, "HI", 2);
}

static const int kMaxQueueSize = 128;
int main(int argc, char *argv[]) {
  if (argc > 2) {
    cerr << "Usage: " << argv[0] << " [<port>]" << endl;
    return kWrongArgumentCount;
  }
  
  unsigned short port = extractPort(argv[1]);
  if (port == kIllegalPort) {
    cerr << "Error: Argument must be purely numeric " 
	 << "and within range [1024, " << kIllegalPort << ")" << endl;
    cerr << "Aborting... " << endl;
    return kIllegalPortArgument;
  }

  int serverSocket = createServerSocket(port, kMaxQueueSize);
  if (serverSocket == kServerSocketFailure) {
    cerr << "Error: Could not start time server to listen to port " << port << "." << endl;
    cerr << "Aborting... " << endl;
    return kServerStartFailure;
  }
  
  cout << "Server listening on port " << port << "." << endl;
  while (true) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    memset(&clientAddress, 0, clientAddressSize);
    int clientSocket = 
      accept(serverSocket, (struct sockaddr *) &clientAddress, 
	     &clientAddressSize);
    cout << "Received a connection request from " 
	 << inet_ntoa(clientAddress.sin_addr) << "." << endl;
    sendData(clientSocket);
  }
  
  return 0; // execution never gets here, but g++ might not know that
}
