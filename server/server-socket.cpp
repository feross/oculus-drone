/**
 * File: server-socket.cc
 * ----------------------
 * Presents the implementation of the
 * createServerSocket function as described in
 * server-socket.h
 */

#include "server-socket.h"
#include <unistd.h>                // for close
#include <sys/socket.h>            // for socket, bind, accept, listen, etc.
#include <arpa/inet.h>             // for htonl, htons, etc.
#include <cstring>                 // for memset

static const int kReuseAddresses = 1;
int createServerSocket(unsigned short port, int backlog) {
  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) return kServerSocketFailure;
  if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
		 &kReuseAddresses, sizeof(int)) < 0) {
    close(serverSocket);
    return kServerSocketFailure;
  }
  
  struct sockaddr_in serverAddress; // IPv4-style socket address
  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress.sin_port = htons(port);

  if (bind(serverSocket, (struct sockaddr *)&serverAddress, 
	   sizeof(serverAddress)) < 0) {
    close(serverSocket);
    return kServerSocketFailure;
  }

  if (listen(serverSocket, backlog) < 0) {
    close(serverSocket);
    return kServerSocketFailure;
  }
  
  return serverSocket;
}
