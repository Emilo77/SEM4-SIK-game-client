#ifndef ZADANIE02_CLIENT_H
#define ZADANIE02_CLIENT_H

#include "Buffer.h"
#include "ClientParameters.h"
#include "GameInfo.h"
#include "Messages.h"
#include "common.h"

#include <iostream>
#include <thread>
#include <mutex>

static bool finish = false;

static void end_program() { finish = true; }

static void catch_int(int sig) {
	finish = true;
	fprintf(stderr, "Signal %d catched. Closing client.\n", sig);
}

class GuiToServerHandler {
public:
	GuiToServerHandler(GameInfo &game_info, ClientParameters &parameters,
	                   int gui_socket, int server_socket, sockaddr_in &address)
			: server_address(address),
			  server_socket(server_socket),
			  gui_socket_recv(gui_socket),
			  game_info(game_info),
			  parameters(parameters) {
		run();
	}


private:
	void run();

	void receive();

	std::optional<size_t> handle_received_message();

	ClientMessageToServer
	prepare_msg_to_server(DisplayMessageToClient &message);

	void send_to_server(size_t send_length);

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
	ServerToGuiHandler(GameInfo &game_info, ClientParameters &parameters,
	                   int gui_socket_send, int server_socket,
	                   sockaddr_in gui_address)
			: gui_address_send(gui_address),
			  server_socket(server_socket),
			  gui_socket_send(gui_socket_send),
			  game_info(game_info),
			  parameters(parameters) {
		run();
	}

private:

	void run();

	void receive();

	std::optional<size_t> handle_message_from_server();

	bool should_notify_display(ServerMessageToClient &message);

	ClientMessageToDisplay prepare_msg_to_display();

	void send_to_display(size_t send_length);


	sockaddr_in gui_address_send;
	int server_socket;
	int gui_socket_send;
	GameInfo &game_info;
	ClientParameters parameters;
	ssize_t received_length{0};
	Buffer buffer;
};

class Client {

public:
	explicit Client(ClientParameters &parameters) : parameters(parameters) {
		initialize();
	}

	void run();

private:
	void initialize();

	ClientParameters parameters;
	GameInfo game_info;
	struct sockaddr_in gui_address{};
	struct sockaddr_in server_address{};
	int gui_socket_recv{-1};
	int gui_socket_send{-1};
	int server_socket{-1};
};


#endif //ZADANIE02_CLIENT_H
