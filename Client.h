#ifndef ZADANIE02_CLIENT_H
#define ZADANIE02_CLIENT_H

#include "GameInfo.h"
#include "Buffer.h"
#include "Messages.h"

#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

/* Obsługa sygnałów */
static void handler(const boost::system::error_code &error, int signal_number) {
	if (error) {
		std::cerr << "Error while handling signal: " << error.message()
		          << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cerr << "Signal " << signal_number << " received." << std::endl;
	std::cerr << "Thanks for using the client!" << std::endl;

	exit(EXIT_SUCCESS);
}

class Client {

public:
	explicit Client(ClientParameters &parameters) : parameters(parameters) {
		initialize();
	}

	/* Uruchomienie klienta. */
	void run() {
		signals.async_wait(handler);
		do_receive_from_server();
		do_receive_from_gui();
		io_context.run();
	}

private:
	void initialize();

	void exit_program(int status);

/* Funkcje do połączenia GUI -> CLIENT -> SERVER */

	/* Odebranie wiadomości od gui */
	void do_receive_from_gui();

	/* Obsługa wiadomości */
	void do_handle_gui();

	/* Obsługa wiadomości */
	std::optional<size_t> handle_gui_message();

	/* Przygotowanie wiadomości do wysłania */
	ClientMessageToServer
	prepare_msg_to_server(DisplayMessageToClient &message);

	/* Wysłanie wiadomości do serwera */
	void do_send_server(size_t send_length);

/* Funkcje do połączenia SERVER -> CLIENT -> GUI */

	/* Odebranie wiadomości od serwera */
	void do_receive_from_server();

	/* Obsługa wiadomości */
	void do_handle_server();

	/* Obsługa wiadomości */
	std::optional<size_t> handle_message_from_server();

	/* Przygotowanie wiadomości do wysłania */
	std::optional<ClientMessageToDisplay>
	prepare_msg_to_gui(ServerMessageToClientType type);

	/* Wysłanie wiadomości do gui */
	void do_send_gui(size_t send_length);

private:
	size_t received_length{0};
	ClientParameters parameters;
	GameInfo game_info;
	Buffer buffer;
	boost::asio::io_context io_context;
	std::optional<udp::socket> gui_socket;
	std::optional<tcp::socket> server_socket;
	std::optional<udp::endpoint> gui_endpoints;
	std::optional<tcp::endpoint> server_endpoints;
	boost::asio::signal_set signals{io_context, SIGINT};
};


#endif //ZADANIE02_CLIENT_H