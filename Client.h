#ifndef ZADANIE02_CLIENT_H
#define ZADANIE02_CLIENT_H

#include "ClientParameters.h"
#include "Utils.h"


class Client {
private:
	ClientParameters parameters;
public:
	explicit Client(ClientParameters &parameters) : parameters(parameters) {}

	DrawMessage handle_received_message();

	void initialize();

	void run();
};

//int main(int argc, char *argv[]) {
//	ClientParameters parameters(argc, argv);
//	Client client(parameters);
//	client.run();
//}


#endif //ZADANIE02_CLIENT_H
