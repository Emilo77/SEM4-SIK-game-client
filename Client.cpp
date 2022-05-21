#include "Client.h"

void Client::run() {
	std::cout << "Client is running" << std::endl;
//	GameInfo gameInfo = game_info;
	std::thread t1{[this]() {
		GuiToServerHandler handler(this->game_info, this->parameters,
		                           this->gui_socket_recv, this->server_socket,
		                           this->server_address);
	}};
	std::thread t2{[]() { while (!finish) {}}};

	t1.join();
	t2.join();
	std::cout << "Ending main thread" << std::endl;
}

//void Client::initialize() {

int main(int argc, char *argv[]) {
	install_signal_handler(SIGINT, catch_int, SA_RESTART);
	ClientParameters parameters(argc, argv);
	Client client(parameters);
	client.run();
}



