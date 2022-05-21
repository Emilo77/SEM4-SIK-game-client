#ifndef MIMUW_SIK_TCP_SOCKETS_COMMON_H
#define MIMUW_SIK_TCP_SOCKETS_COMMON_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cstdint>
#include <arpa/inet.h>
#include <csignal>
#include "err.h"

#define NO_FLAGS 0

enum ConnectionType {
	TCP,
	UDP,
};


inline static struct sockaddr_in
get_address(char *host, uint16_t port, ConnectionType connection) {
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET; // IPv4
	switch (connection) {
		case TCP:
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			break;
		case UDP:
			hints.ai_socktype = SOCK_DGRAM;
			hints.ai_protocol = IPPROTO_UDP;
			break;
	}

	struct addrinfo *address_result;
	CHECK(getaddrinfo(host, nullptr, &hints, &address_result));

	struct sockaddr_in address;
	address.sin_family = AF_INET; // IPv4, zmienić
	address.sin_addr.s_addr =
			((struct sockaddr_in *) (address_result->ai_addr))->sin_addr.s_addr; // IP address
	address.sin_port = htons(port);

	freeaddrinfo(address_result);

	return address;
}

inline static int open_tcp_socket() {
	int socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_fd < 0) {
		PRINT_ERRNO();
	}

	return socket_fd;
}

inline static int open_udp_socket() {
	int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0) {
		PRINT_ERRNO();
	}

	return socket_fd;
}

inline static void
connect_socket(int socket_fd, const struct sockaddr_in *address) {
	int result = connect(socket_fd, (struct sockaddr *) address,
	                     sizeof(*address));
	if (result == -1) {
		std::cerr << "Couldn't connect to server" << std::endl;
		std::cerr << "Consider changing the server address" << std::endl;
	}

}


#endif //MIMUW_SIK_TCP_SOCKETS_COMMON_H
