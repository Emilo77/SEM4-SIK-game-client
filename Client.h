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
	GameInfo game_info;
	bool finish{false};

	void send_to_display();

	void send_to_server();

	void receive_from_dislay();

	void receive_from_server();

	size_t put_display_msg_to_buffer(ClientMessageToDisplay &drawMessage);

	size_t put_server_msg_to_buffer(ClientMessageToServer &message);

	std::optional<DisplayMessageToClient>
	get_display_msg_from_buffer(size_t length);

	std::optional<ServerMessageToClient>
	get_server_msg_from_buffer(size_t length);

	ClientMessageToDisplay prepare_msg_to_display();

	ClientMessageToServer prepare_msg_to_server(DisplayMessageToClient &message);


	void handle_message_from_server(size_t length);

	void handle_message_from_display(size_t length);

	void bind_sockets();

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
