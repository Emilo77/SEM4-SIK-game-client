#include "Client.h"

void Client::run() {

	std::cerr << "Client is running" << std::endl;
	std::thread t1{[this]() {
		GuiToServerHandler handler(this->game_info,
		                           this->parameters,
		                           this->gui_socket_recv,
		                           this->server_socket,
		                           this->server_address);

	}};
	std::thread t2{[this]() {
		ServerToGuiHandler handler(this->game_info,
		                           this->parameters,
		                           this->gui_socket_send,
		                           this->server_socket,
		                           this->gui_address);
	}};

	close(server_socket);
	close(gui_socket_recv);
	close(gui_socket_send);
	t1.join();
	t2.join();

	std::cerr << "Ending main thread" << std::endl;
}

void Client::initialize() {
	gui_address = get_address(parameters.gui_host, parameters.gui_port,
	                          UDP);
	server_address = get_address(parameters.server_host,
	                             parameters.server_port, TCP);

	gui_socket_recv = open_udp_socket();
	gui_socket_send = open_udp_socket();
	server_socket = open_tcp_socket();
	connect_socket(server_socket, &server_address);
}

int main(int argc, char *argv[]) {
	install_signal_handler(SIGINT, catch_int, SA_RESTART);
	ClientParameters parameters(argc, argv);
	Client client(parameters);
	client.run();
}



