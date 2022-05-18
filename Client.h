#ifndef ZADANIE02_CLIENT_H
#define ZADANIE02_CLIENT_H

#include "Buffer.h"
#include "ClientParameters.h"
#include "GameInfo.h"
#include "Messages.h"
#include <iostream>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include <utility>

using boost::asio::ip::udp;
using boost::asio::ip::tcp;


class GuiToServerHandler {
public:
	GuiToServerHandler(GameInfo &game_info, ClientParameters &parameters,
	                   boost::asio::io_context &io_context,
	                   tcp::resolver::results_type server_endpoint,
	                   udp::resolver::results_type gui_endpoint)
			: game_info(game_info),
			  parameters(parameters),
			  io_context(io_context),
			  socket(io_context),
			  server_endpoint(std::move(server_endpoint)),
			  gui_endpoint(std::move(gui_endpoint)) {}

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
	boost::asio::io_context &io_context;
	tcp::socket socket;
	tcp::resolver::results_type server_endpoint;
	udp::resolver::results_type gui_endpoint;
};

class ServerToGuiHandler {

public:
	ServerToGuiHandler(GameInfo &game_info, ClientParameters &parameters,
	                   boost::asio::io_context &io_context,
	                   tcp::resolver::results_type server_endpoint,
	                   udp::resolver::results_type gui_endpoint)
			: game_info(game_info),
			  parameters(parameters),
			  io_context(io_context),
			  socket(io_context),
			  server_endpoint(std::move(server_endpoint)),
			  gui_endpoint(std::move(gui_endpoint)) {}

private:

	void connect_with_server();

	void receive_from_server();

	void handle_message_from_server(const boost::system::error_code &error,
	                                size_t length);

	ClientMessageToDisplay prepare_msg_to_display();

	void send_to_display();


	Buffer buffer;
	GameInfo &game_info;
	ClientParameters parameters;
	boost::asio::io_context &io_context;
	tcp::socket socket;
	tcp::resolver::results_type server_endpoint;
	udp::resolver::results_type gui_endpoint;
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
	bool finish{false};
	boost::asio::io_context io_context;
	std::optional<GuiToServerHandler> handler_to_server;
	std::optional<ServerToGuiHandler> handler_to_gui;
};

int main(int argc, char *argv[]) {
	ClientParameters parameters(argc, argv);
	Client client(parameters);
	client.run();
}


#endif //ZADANIE02_CLIENT_H
