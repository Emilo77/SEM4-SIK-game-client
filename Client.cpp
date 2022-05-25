#include "Client.h"

void Client::exit_program(int status) {
	if (server_socket.value().is_open()) {
		server_socket.value().close();
	}
	if (gui_socket.value().is_open()) {
		gui_socket.value().close();
	}
	exit(status);
}


void Client::initialize() {
	try {
		/* Wyznaczamy endpointy GUI na podstawie hosta i portu GUI. */
		udp::resolver gui_resolver(io_context);
		udp::endpoint gui_endpoints = *gui_resolver.resolve(
				parameters.gui_host,
				boost::lexical_cast<std::string>(parameters.gui_port));

		/* Wyznaczamy endpointy GUI na podstawie hosta i portu GUI. */
		tcp::resolver tcp_resolver(io_context);
		tcp::endpoint tcp_endpoints = *tcp_resolver.resolve(
				parameters.server_host,
				boost::lexical_cast<std::string>(parameters.server_port));

		/* Gniazdo GUI będzie nasłuchiwać na porcie podanym w parametrach */
		gui_socket.emplace(io_context,
		                   udp::endpoint(udp::v6(), parameters.port));
		server_socket.emplace(io_context);

		/* Łączymy gniazda z odpowiednimi endpointami */
		server_socket.value().connect(tcp_endpoints);
		gui_socket.value().connect(gui_endpoints);

		/* Wyłączamy algorytm Nagle'a */
		server_socket.value().set_option(tcp::no_delay(true));

		/* Dodanie obsługi zakończenia programu przez sygnał SIGINT */
		boost::asio::signal_set signals(io_context, SIGINT);
		signals.async_wait(handler);

	}
	/* W przypadku błędu wypisujemy błąd i kończymy działanie programu */
	catch (std::exception &e) {
		std::cerr << "Exception: " << e.what() << "\n";
		exit_program(EXIT_FAILURE);
	}
}

std::optional<size_t> Client::handle_gui_message() {

	/* Wyciągamy i przetwarzamy wiadomość z bufora. */
	auto message = buffer.receive_msg_from_display((size_t) received_length);

	/* Jeżeli wiadomość od gui jest poprawna, przekazujemy dalej do serwera. */
	if (message.has_value()) {
		/* Na podstawie otrzymanej wiadomości, tworzymy nową wiadomość
		 * do serwera, tę wiadomość wstawiamy do bufora. */
		ClientMessageToServer reply = prepare_msg_to_server(message.value());
		return buffer.insert_msg_to_server(reply);
	}

/* Jeżeli wiadomość od GUI przyszła w niepoprawnym formacie, ignorujemy ją. */
	return {};
}

ClientMessageToServer
Client::prepare_msg_to_server(DisplayMessageToClient &message) {
	ClientMessageToServer new_message;

	/* Sprawdzamy, czy gra jest w stanie Lobby, czy Gameplay.
	 * Jeżeli Gameplay, tworzymy odpowiednie komunikaty na podstawie
	 * komunikatu od GUI.
	 * Jeżeli w stanie Lobby, zawsze tworzymy komunikat Join. */
	if (game_info.is_gameplay()) {
		switch (message.type) {
			case PlaceBombDisplay:
				new_message.type = ClientMessageToServerType::PlaceBombServer;
				break;
			case PlaceBlockDisplay:
				new_message.type = ClientMessageToServerType::PlaceBlockServer;
				break;
			case MoveDisplay:
				new_message.type = ClientMessageToServerType::MoveServer;
				new_message.data = message.direction;
				break;
		}
	} else {
		new_message.type = ClientMessageToServerType::JoinServer;
		new_message.data = parameters.player_name;
	}
	return new_message;
}

void Client::do_receive_from_gui() {
	gui_socket.value().async_receive(
			boost::asio::buffer(buffer.get(), BUFFER_SIZE),
			[this](boost::system::error_code ec,
			       std::size_t length) {
				received_length = length;
				do_handle_gui(ec);
			});
}

void Client::do_handle_gui(const boost::system::error_code &ec) {
	if (!ec) {
		auto send = handle_gui_message();
		if (send.has_value()) {
			do_send_server(send.value());
		}
	} else {
		std::cout << "Error: " << ec.message() << std::endl;
		exit_program(EXIT_FAILURE);
	}
}


void Client::do_send_server(size_t send_length) {
	server_socket.value().async_send(
			boost::asio::buffer(buffer.get(), send_length),
			[this](boost::system::error_code ec,
			       std::size_t length) {
				if (!ec) {
					do_receive_from_gui();
				} else {
				}
			});
}


std::optional<size_t> Client::handle_message_from_server() {
	if (received_length > 0) {
		/* Wyciągamy i przetwarzamy wiadomość z bufora. */
		auto message = buffer.receive_msg_from_server((size_t) received_length);
		if (message.has_value()) {
			/* Jeżeli wiadomość jest poprawna, aktualizujemy stan gry. */
			game_info.apply_changes_from_server(message.value());

			/* Jeżeli otrzymaliśmy komunikat PlayerAccepted lub Turn,
			 * wysyłamy odpowiedź do GUI.*/
			if (should_notify_display(message.value())) {

				/* Tworzymy nową wiadomość do serwera,
				 * tę wiadomość wstawiamy do bufora. */
				ClientMessageToDisplay reply = prepare_msg_to_display();
				return buffer.insert_msg_to_display(reply);
			}
		} else {
			/* Jeżeli wiadomość jest w niepoprawnym formacie, rozłączamy się
			 * i kończymy działanie programu. */
			std::cerr << "Error: received message from server is not valid.\n";
			exit_program(EXIT_FAILURE);
		}
	} else if (received_length == 0) {
		/* Jeżeli rozmiar otrzymanej wiadomości wynosi 0, rozłączamy się
		 * i kończymy działanie programu. */
		std::cerr << "Connection with server closed.\n";
		exit_program(EXIT_SUCCESS);
	} else {
		/* Jeżeli rozmiar otrzymanej wiadomości jest niepoprawny,
		 * rozłączamy się i kończymy działanie programu. */
		std::cerr << "Error: received message from server is not valid.\n";
		exit_program(EXIT_FAILURE);
	}
	return {};
}


ClientMessageToDisplay Client::prepare_msg_to_display() {
	/* Tworzymy wiadomość na podstawie aktualnego stanu gry */
	if (!game_info.is_gameplay()) {
		return {GameState::LobbyState, game_info.create_lobby_msg()};
	}
	return {GameState::GameplayState, game_info.create_gameplay_msg()};
}

bool Client::should_notify_display(ServerMessageToClient &message) {
	return (message.type == AcceptedPlayer && !game_info.is_gameplay())
	       || (message.type == Turn && game_info.is_gameplay());
}

void Client::do_receive_from_server() {
	server_socket.value().async_receive(
			boost::asio::buffer(buffer.get(), BUFFER_SIZE),
			[this](boost::system::error_code ec,
			       std::size_t length) {
				received_length = length;
				do_handle_server(ec);
			});
}

void Client::do_handle_server(const boost::system::error_code &ec) {
	if (!ec) {
		auto send = handle_message_from_server();
		if (send.has_value()) {
			do_send_gui(send.value());
		}
	} else {
		std::cout << "Error: " << ec.message() << std::endl;
		exit_program(EXIT_FAILURE);
	}
}

void Client::do_send_gui(size_t send_length) {
	gui_socket.value().async_send(
			boost::asio::buffer(buffer.get(), send_length),
			[this](boost::system::error_code ec,
			       std::size_t length) {
				if (!ec) {
					do_receive_from_server();

				} else {
					exit_program(EXIT_FAILURE);
				}
			});
}

int main(int argc, char *argv[]) {
	ClientParameters parameters(argc, argv);
	Client client(parameters);
	client.run();
}
