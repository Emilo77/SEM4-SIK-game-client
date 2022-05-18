#include "Client.h"

void Client::run() {
	std::cout << "Client is running" << std::endl;

	io_context.run();

	std::cout << "Nothing to do, client closed" << std::endl;
}

void Client::initialize() {
	tcp::resolver tcp_resolver(Client::io_context);
	udp::resolver udp_resolver(Client::io_context);
	//todo może do zmiany
	try {
		tcp::resolver::results_type server_endpoint = tcp_resolver.resolve(
				parameters.server_ip);
		udp::resolver::results_type gui_endpoint = udp_resolver.resolve(
				parameters.display_ip);


		handler_to_gui.emplace(game_info, parameters, io_context,
		                       server_endpoint,
		                       gui_endpoint);
		handler_to_server.emplace(game_info, parameters, io_context,
		                          server_endpoint,
		                          gui_endpoint);


	} catch (std::exception &e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

int main(int argc, char *argv[]) {
	ClientParameters parameters(argc, argv);
	Client client(parameters);
	client.run();
}



