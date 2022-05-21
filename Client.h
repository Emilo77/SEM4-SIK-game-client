#ifndef ZADANIE02_CLIENT_H
#define ZADANIE02_CLIENT_H

#include "Buffer.h"
#include "ClientParameters.h"
#include "GameInfo.h"
#include "Messages.h"
#include "common.h"

#include <iostream>
#include <thread>

bool finish = false;

void end_program() { finish = true; }

static void catch_int(int sig) {
	finish = true;
	fprintf(stderr, "Signal %d catched. Closing client.\n", sig);
}

inline static void
install_signal_handler(int signal, void (*handler)(int), int flags) {
	struct sigaction action{};
	sigset_t block_mask;

	sigemptyset(&block_mask);
	action.sa_handler = handler;
	action.sa_mask = block_mask;
	action.sa_flags = flags;

	CHECK_ERRNO(sigaction(signal, &action, nullptr));
}

class GuiToServerHandler {
public:
	GuiToServerHandler(GameInfo &game_info, ClientParameters &parameters,
	                   int gui_socket, int server_socket, sockaddr_in address)
			: server_address(address),
			  server_socket(server_socket),
			  gui_socket_recv(gui_socket),
			  game_info(game_info),
			  parameters(parameters) {
		run();
	}


private:
	void run();

	void receive() {
		received_length = read(gui_socket_recv, buffer.get(), BUFFER_SIZE);
		if (received_length < 0) {
			fprintf(stderr,
			        "Error when reading message from server (errno %d,%s)\n",
			        errno, strerror(errno));
			end_program();
		} else if (received_length == 0) {
			end_program();
		}
	}

	std::optional<size_t> handle_received_message();

	ClientMessageToServer
	prepare_msg_to_server(DisplayMessageToClient &message);

	void send_to_server(size_t send_length) {
		send_message_to(server_socket, &server_address, buffer.get(),
		                send_length);
	}

private:
	sockaddr_in server_address;
	int server_socket;
	int gui_socket_recv;
	GameInfo &game_info;
	ClientParameters parameters;
	ssize_t received_length{0};
	Buffer buffer;
};

class ServerToGuiHandler {

public:
	ServerToGuiHandler(GameInfo &game_info, ClientParameters &parameters)
			: game_info(game_info),
			  parameters(parameters) {}

private:

	void receive();


	void handle_message_from_server(const boost::system::error_code &error,
	                                size_t length);

	bool should_notify_display(ServerMessageToClient &message);

	ClientMessageToDisplay prepare_msg_to_display();

	void send_to_display();


	Buffer buffer;
	GameInfo &game_info;
	ClientParameters parameters;
};

class Client {

public:
	explicit Client(ClientParameters &parameters) : parameters(parameters) {
		initialize();
	}

	void run();

private:
	void initialize() {
		gui_address = get_address(parameters.gui_host, parameters.gui_port,
		                          UDP);
		server_address = get_address(parameters.server_host,
		                             parameters.server_port, TCP);

		gui_socket_recv = open_udp_socket();
		gui_socket_send = open_udp_socket();
		server_socket = open_tcp_socket();
		connect_socket(server_socket, &server_address);
	}


	ClientParameters parameters;
	GameInfo game_info;
	struct sockaddr_in gui_address;
	struct sockaddr_in server_address;
	int gui_socket_recv{-1};
	int gui_socket_send{-1};
	int server_socket{-1};
};


#endif //ZADANIE02_CLIENT_H
