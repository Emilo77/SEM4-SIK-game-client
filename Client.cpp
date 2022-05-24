#include "Client.h"
#include "err.h"

void Client::run() {


	std::cerr << "Ending main thread" << std::endl;
}

int main(int argc, char *argv[]) {
//	install_signal_handler(SIGINT, catch_int, SA_RESTART);
	ClientParameters parameters(argc, argv);
	Client client(parameters);
}




