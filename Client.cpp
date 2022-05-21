#include "Client.h"

void Client::run() {
	std::cout << "Client is running" << std::endl;

//	io_context.run();

	std::cout << "Nothing to do, client closed" << std::endl;
}

//void Client::initialize() {
//}

int main(int argc, char *argv[]) {
	ClientParameters parameters(argc, argv);
	Client client(parameters);
	client.run();
}



