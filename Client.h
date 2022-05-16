#ifndef ZADANIE02_CLIENT_H
#define ZADANIE02_CLIENT_H

#include "Buffer.h"
#include "ClientParameters.h"
#include "GameInfo.h"
#include <iostream>

class Client {
private:
	ClientParameters parameters;
	Buffer buffer;
	GameInfo gameInfo;
	bool finish{false};

	void bind_sockets();

	void receive_from_dislay();

	void receive_from_server();

	void send_to_display();

	void send_to_server();

	void handle_message_from_server(size_t length);

	void handle_message_from_display(size_t length);

	void initialize();

public:
	explicit Client(ClientParameters &parameters) : parameters(parameters) {
		initialize();
	}
	void run();
};

int main(int argc, char *argv[]) {
	ClientParameters parameters(argc, argv);
	Client client(parameters);
	client.run();
}


#endif //ZADANIE02_CLIENT_H
