#ifndef ZADANIE02_CLIENT_H
#define ZADANIE02_CLIENT_H

#include "Buffer.h"
#include "ClientParameters.h"
#include "GameInfo.h"
#include "Utils.h"

class Client {
private:
	ClientParameters parameters;
	Buffer buffer;
	GameInfo gameInfo;

public:
	explicit Client(ClientParameters &parameters) : parameters(parameters) {}

	ServerMessage handle_message_from_server();

	DrawMessage handle_message_from_display();

	void initialize();

	void run();
};

//int main(int argc, char *argv[]) {
//	ClientParameters parameters(argc, argv);
//	Client client(parameters);
//	client.run();
//}


#endif //ZADANIE02_CLIENT_H
