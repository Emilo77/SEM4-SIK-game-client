#include "Client.h"

void Client::exit_program(int status) {
	/* Zamykamy wszystkie otwarte gniazda. */
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
		udp::endpoint new_gui_endpoints = *gui_resolver.resolve(
				parameters.gui_host,
				boost::lexical_cast<std::string>(parameters.gui_port));

		/* Wyznaczamy endpointy GUI na podstawie hosta i portu GUI. */
		tcp::resolver server_resolver(io_context);
		tcp::endpoint new_server_endpoints = *server_resolver.resolve(
				parameters.server_host,
				boost::lexical_cast<std::string>(parameters.server_port));

		gui_endpoints.emplace(new_gui_endpoints);
		server_endpoints.emplace(new_server_endpoints);

		/* Gniazdo GUI będzie nasłuchiwać na porcie podanym w parametrach */
		gui_socket.emplace(io_context,
		                   udp::endpoint(udp::v6(), parameters.port));
		server_socket.emplace(io_context);

		/* Łączymy gniazda z odpowiednimi endpointami */
		server_socket.value().connect(server_endpoints.value());
//		gui_socket.value().connect(gui_endpoints);

		/* Wyłączamy algorytm Nagle'a */
		server_socket.value().set_option(tcp::no_delay(true));
	}
	/* W przypadku błędu wypisujemy błąd i kończymy działanie programu */
	catch (std::exception &e) {
		std::cerr << "Exception: " << e.what() << "\n";
		exit_program(EXIT_FAILURE);
	}
}

std::optional<size_t> Client::handle_gui_message() {
	std:: cerr << "weszło do przetwarzania wiadomości GUI\n";

	/* Wyciągamy i przetwarzamy wiadomość z bufora. */
	auto message = buffer.receive_msg_from_gui((size_t) received_length);
	buffer.print(received_length);

	/* Jeżeli wiadomość od gui jest poprawna, przekazujemy dalej do serwera. */
	if (message.has_value()) {
		std::cerr << "WESZŁO TU ESSSS\n";
		/* Na podstawie otrzymanej wiadomości, tworzymy nową wiadomość
		 * do serwera, tę wiadomość wstawiamy do bufora. */
		std:: cerr << message.value().type << "\n";
		std:: cerr << (int) message.value().direction.value() << "\n";
		ClientMessageToServer reply = prepare_msg_to_server(message.value());
		std::cerr << "WESZŁO TU ESSSS 2 \n";
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
				std:: cerr << "weszło do PlaceBombDisplay\n";
				new_message.type = ClientMessageToServerType::PlaceBombServer;
				break;
			case PlaceBlockDisplay:
				std:: cerr << "weszło do PlaceBlockDisplay\n";
				new_message.type = ClientMessageToServerType::PlaceBlockServer;
				break;
			case MoveDisplay:
				std:: cerr << "weszło do MoveDisplay\n";
				new_message.type = ClientMessageToServerType::MoveServer;
				std:: cerr << "weszło do MoveDisplay 2\n";
				new_message.data = message.direction.value();
				std:: cerr << "weszło do MoveDisplay 3\n";
				break;
		}
	} else {
		std:: cerr << "weszło do else\n";
		new_message.type = ClientMessageToServerType::JoinServer;
		new_message.data = parameters.player_name;
	}
	return new_message;
}

void Client::do_receive_from_gui() {
	std:: cerr << "Weszło do początku odbierania\n";
	gui_socket.value().async_receive(
			boost::asio::buffer(buffer.get(), BUFFER_SIZE),
			[this](boost::system::error_code ec,
			       std::size_t length) {
				received_length = length;
				std:: cerr << "Weszło do handlera\n";
				buffer.print(received_length);
				do_handle_gui(ec);
			});
}

void Client::do_handle_gui(const boost::system::error_code &ec) {
	std:: cerr << "Weszło do do_handle_gui\n";
	if (!ec) {
		auto send = handle_gui_message();
		if (send.has_value()) {
			buffer.print(send.value());
			std:: cerr << "Zaraz wejdzie do do_send_server\n";
			do_send_server(send.value());
		} else {
			do_receive_from_gui();
		}
	} else {
		std::cerr << "Error receiving from gui: " << ec.message() << std::endl;
		exit_program(EXIT_FAILURE);
	}
}


void Client::do_send_server(size_t send_length) {
	std:: cerr << "Weszło do do_send_server\n";
	server_socket.value().async_send(
			boost::asio::buffer(buffer.get(), send_length),
			[this](boost::system::error_code ec,
			       std::size_t length) {
				if (!ec) {
					std:: cerr << "Wysłano do servera \n";
					do_receive_from_gui();
				} else {
				}
			});
}

std::optional<size_t> Client::handle_message_from_server() {
	if (received_length > 0) {
		buffer.print(received_length);
		/* Wyciągamy i przetwarzamy wiadomość z bufora. */
		auto message = buffer.receive_msg_from_server();
		if (message.has_value()) {
			/* Jeżeli wiadomość jest poprawna, aktualizujemy stan gry. */
			game_info.apply_changes_from_server(message.value());

				/* Na podstawie otrzymanego komunikatu, odpowiadamy GUI:
				 * Turn: wysyłamy Game
				 * AcceptedPlayer, GameEnded, Hello: wysyłamy Lobby
				 * GameStarted: nie wysyłamy nic */
				auto reply = prepare_msg_to_gui(message.value().type);
				if (reply.has_value()) {
					return buffer.insert_msg_to_display(reply.value());
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


std::optional<ClientMessageToDisplay> Client::prepare_msg_to_gui(ServerMessageToClientType type) {
	std::optional<ClientMessageToDisplay> reply;
	/* Tworzymy wiadomość na podstawie aktualnego stanu gry */
	if (type == ServerMessageToClientType::GameStarted) {
		return {};
	} else if (type == ServerMessageToClientType::Turn) {
		reply.emplace(GameState::GameplayState, game_info.create_gameplay_msg());
	} else {
		reply.emplace(GameState::LobbyState, game_info.create_lobby_msg());
	}
	return reply;
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
		std::cerr << " chuj " << std::endl;
		auto send = handle_message_from_server();
		if (send.has_value()) {
			std::cerr << " chuj2 " << std::endl;
			do_send_gui(send.value());
		} else {
			do_receive_from_server();
		}
	} else {
		std::cerr << "Error receiving from server: " << ec.message() << std::endl;

		exit_program(EXIT_FAILURE);
	}
}

void Client::do_send_gui(size_t send_length) {
	std:: cerr << " chuj3 " << std::endl;
	buffer.print(send_length);
	gui_socket.value().async_send_to(
			boost::asio::buffer(buffer.get(), send_length),
			gui_endpoints.value(),
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
