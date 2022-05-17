#ifndef ZADANIE02_CLIENT_H
#define ZADANIE02_CLIENT_H

#include "Buffer.h"
#include "ClientParameters.h"
#include "GameInfo.h"
#include "Messages.h"
#include <iostream>

#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;
using boost::asio::ip::tcp;
boost::asio::io_context io_context;

class Client {

public:
	explicit Client(ClientParameters &parameters) : parameters(parameters),
	display_socket(io_context, udp::endpoint(udp::v6(), parameters.port)), {}

	void run();

private:
private:
	ClientParameters parameters;
	Buffer buffer;
	GameInfo game_info;
	bool finish{false};
	udp::socket display_socket;
	udp::endpoint display_endpoint;
//	tcp::socket server_socket;
	tcp::endpoint server_endpoint;


	void connect_display();

	void connect_server();

	void send_to_display();

	void send_to_server();

	void receive_from_dislay();

	void receive_from_server();

	ClientMessageToDisplay prepare_msg_to_display();

	ClientMessageToServer prepare_msg_to_server(DisplayMessageToClient &message);


	void handle_message_from_server(const boost::system::error_code& error,
	                                size_t length);

	void handle_message_from_display(const boost::system::error_code& error,
	                                 size_t length);

	void initialize();


};

int main(int argc, char *argv[]) {
	ClientParameters parameters(argc, argv);
	Client client(parameters);
	client.run();
}


#endif //ZADANIE02_CLIENT_H
