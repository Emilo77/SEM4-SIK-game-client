#ifndef ZADANIE02_CLIENT_H
#define ZADANIE02_CLIENT_H

#include "Handlers.h"

#include <iostream>
#include <thread>


//static bool finish = false;
//
//static void end_program() { finish = true; }
//
//static void catch_int(int sig) {
//	finish = true;
//	fprintf(stderr, "Signal %d catched. Closing client.\n", sig);
//}

class Client {

public:
	explicit Client(ClientParameters &parameters) : parameters(parameters) {
		try {
			udp::resolver gui_resolver(io_context);
			udp::endpoint gui_endpoints = *gui_resolver.resolve(
					parameters.gui_host,
					boost::lexical_cast<std::string>(parameters.gui_port));


			tcp::resolver tcp_resolver(io_context);
			tcp::endpoint tcp_endpoints = *tcp_resolver.resolve(
					parameters.server_host,
					boost::lexical_cast<std::string>(parameters.server_port));


			udp::socket gui_socket(io_context,
			                       udp::endpoint(udp::v6(), parameters.port));
			tcp::socket server_socket(io_context);

			server_socket.connect(tcp_endpoints);
			gui_socket.connect(gui_endpoints);

			gui_to_server_handler.emplace(
					GuiToServerHandler(game_info, parameters, server_socket,
					                   gui_socket));

			server_to_gui_handler.emplace(
					ServerToGuiHandler(game_info, parameters, server_socket,
					                   gui_socket));

		}
		catch (std::exception &e) {
			std::cerr << "Exception: " << e.what() << "\n";
		}

	}

	/* Uruchomienie klienta */
	void run() {
		if (gui_to_server_handler.has_value() && server_to_gui_handler.has_value()) {
			gui_to_server_handler.value().run();
//			server_to_gui_handler.value().run();
			io_context.run();
			std::cerr << "Client is running" << std::endl;
		}
	}

private:
	ClientParameters parameters;
	GameInfo game_info;
	boost::asio::io_context io_context;
	std::optional<ServerToGuiHandler> server_to_gui_handler;
	std::optional<GuiToServerHandler> gui_to_server_handler;

	bool finish{false};
};




#endif //ZADANIE02_CLIENT_H
