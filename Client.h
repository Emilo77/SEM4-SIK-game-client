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
					parameters.gui_address,
					boost::lexical_cast<std::string>(parameters.gui_port));


			tcp::resolver tcp_resolver(io_context);
			tcp::endpoint tcp_endpoints = *tcp_resolver.resolve(
					parameters.server_address,
					boost::lexical_cast<std::string>(parameters.server_port));


			udp::socket gui_sender(io_context);
			udp::socket gui_receiver(io_context,
			                         udp::endpoint(udp::v6(), parameters.port));

			tcp::socket server_receiver(io_context);

			gui_receiver.open(udp::v6());
			gui_receiver.bind(udp::endpoint(udp::v6(), parameters.port));

			gui_sender.open(udp::v6());

			server_receiver.connect(tcp_endpoints);

		}
		catch (std::exception &e) {
			std::cerr << "Exception: " << e.what() << "\n";
		}
	}

	void run();

private:
	void initialize() {


		gui_sender

	}


	ClientParameters parameters;
	GameInfo game_info;
	boost::asio::io_context io_context;



	bool finish{false};
};


#endif //ZADANIE02_CLIENT_H
