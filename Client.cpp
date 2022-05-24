#include "Client.h"
#include "err.h"

void Client::run() {


	std::cerr << "Ending main thread" << std::endl;
}

int main(int argc, char *argv[]) {
	ClientParameters parameters(argc, argv);
	Client client(parameters);
}




