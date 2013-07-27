/**
 * File: server-socket.h
 * ---------------------
 * Provides a single function that sets up
 * a server socket, binding it to any of the
 * IP addresses associated with the host machine
 * on the specified port.
 */

#ifndef _server_socket_
#define _server_socket_

/**
 * Constant: kServerSocketFailure
 * ------------------------------
 * Constant returned by createServerSocket if the
 * server socket couldn't be created or otherwise
 * bound to listen to the specified port.
 */

const int kServerSocketFailure = -1;

/**
 * Function: createServerSocket
 * ----------------------------
 * createServerSocket creates a server socket to
 * listen for all client connections on the given
 * port.  The function returns a valid server socket
 * descriptor that allows a specified backlog of
 * connection requests to accummulation before
 * additional requests might be refused.
 */

int createServerSocket(unsigned short port, int backlog);

#endif
