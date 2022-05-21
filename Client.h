#ifndef ZADANIE02_CLIENT_H
#define ZADANIE02_CLIENT_H

#include "Buffer.h"
#include "ClientParameters.h"
#include "GameInfo.h"
#include "Messages.h"
#include "common.h"

#include <iostream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <utility>

using boost::asio::ip::udp;
using boost::asio::ip::tcp;


class GuiToServerHandler {
public:
	GuiToServerHandler(GameInfo &game_info, ClientParameters &parameters)
			: game_info(game_info),
			  parameters(parameters) {}

	void connect_with_server();

	void handle_connect(const boost::system::error_code &error);

	void receive_from_dislay();

	void handle_message_from_display(const boost::system::error_code &error,
	                                 size_t length);

	ClientMessageToServer
	prepare_msg_to_server(DisplayMessageToClient &message);

	void send_to_server();

private:
	Buffer buffer;
	GameInfo &game_info;
	ClientParameters parameters;
};

class ServerToGuiHandler {

public:
	ServerToGuiHandler(GameInfo &game_info, ClientParameters &parameters)
			: game_info(game_info),
			  parameters(parameters) {}

private:

	void connect_with_server();

	void receive_from_server();

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
		gui_socket = open_udp_socket();
		server_socket = open_tcp_socket();
//		connect_socket(server_socket, )
	}


	ClientParameters parameters;
	GameInfo game_info;
	bool finish{false};
	int gui_socket{-1};
	int server_socket{-1};
};


#endif //ZADANIE02_CLIENT_H
