#ifndef ZADANIE02_CLIENT_HANDLERS_H
#define ZADANIE02_CLIENT_HANDLERS_H


#include "err.h"
#include "GameInfo.h"
#include "ClientParameters.h"
#include "Buffer.h"
#include "Messages.h"

#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;


//static void
//send_message_to(int socket_fd,
//                const char *message, size_t length) {
//	int flags = 0;
//	ssize_t sent_length =
//			send(socket_fd, message, length, flags);
//	ENSURE(sent_length == (ssize_t) length);
//}


class GuiToServerHandler {
public:
	GuiToServerHandler(GameInfo &game_info, ClientParameters &parameters,
	                   tcp::socket &server_socket,
	                   udp::socket &gui_socket,
	                   udp::endpoint &gui_endpoint)
			:
			game_info(game_info),
			parameters(parameters),
			server_socket(server_socket),
			gui_socket(gui_socket),
			gui_endpoint(gui_endpoint) {}


	void run() {
		do_receive();
	}

private:

	void do_receive();

	void do_handle(const boost::system::error_code &ec);

	std::optional<size_t> hangle_gui_message();

	ClientMessageToServer
	prepare_msg_to_server(DisplayMessageToClient &message);

	void send_to_server(size_t send_length);

private:
	GameInfo &game_info;
	ClientParameters &parameters;
	size_t received_length{0};
	Buffer buffer;
	tcp::socket &server_socket;
	udp::socket &gui_socket;
	udp::endpoint &gui_endpoint;
};

class ServerToGuiHandler {

public:
	ServerToGuiHandler(GameInfo &game_info, ClientParameters &parameters,
	                   tcp::socket &server_socket,
	                   udp::socket &gui_socket,
	                   udp::endpoint &gui_endpoint)
			:
			game_info(game_info),
			parameters(parameters),
			server_socket(server_socket),
			gui_socket(gui_socket),
			gui_endpoint(gui_endpoint) {}

public:

	void run() {
		do_receive();
	}

private:

	void do_receive();

	void do_handle(const boost::system::error_code &ec);

	std::optional<size_t> handle_message_from_server();

	bool should_notify_display(ServerMessageToClient &message);

	ClientMessageToDisplay prepare_msg_to_display();

	void do_send(size_t send_length);


	GameInfo &game_info;
	ClientParameters &parameters;
	size_t received_length{0};
	Buffer buffer;
	tcp::socket &server_socket;
	udp::socket &gui_socket;
	udp::endpoint &gui_endpoint;

};

#endif //ZADANIE02_CLIENT_HANDLERS_H
