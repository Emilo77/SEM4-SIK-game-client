#include "Client.h"

void Client::exit_program(int status) {
	/* Zamykamy gniazdo serwera, jeżeli jest otwarte. */
	if (server_socket.value().is_open()) {
		server_socket.value().close();
	}
	/* Zamykamy gniazdo gui, jeżeli jest otwarte. */
	if (gui_socket.value().is_open()) {
		gui_socket.value().close();
	}
	/* Kończymy działanie programu z odpowiednim kodem wyjścia. */
	exit(status);
}


void Client::initialize() {
	server_to_gui_buffer.initialize();
	gui_to_server_buffer.initialize();
	try {
		/* Wyznaczamy endpointy GUI na podstawie hosta i portu GUI. */
		udp::resolver gui_resolver(io_context);
		udp::endpoint new_gui_endpoints = *gui_resolver.resolve(
				parameters.gui_host,
				boost::lexical_cast<std::string>(parameters.gui_port));

		/* Wyznaczamy endpointy serwera na podstawie hosta i portu serwera. */
		tcp::resolver server_resolver(io_context);
		tcp::endpoint new_server_endpoints = *server_resolver.resolve(
				parameters.server_host,
				boost::lexical_cast<std::string>(parameters.server_port));

		gui_endpoints.emplace(new_gui_endpoints);
		server_endpoints.emplace(new_server_endpoints);

		/* Gniazdo GUI będzie nasłuchiwać na porcie podanym w parametrach */
		gui_socket.emplace(io_context,
		                   udp::endpoint(udp::v6(), parameters.port));
		std::cerr << "GUI socket listening on port " << parameters.port
		          << std::endl;
		server_socket.emplace(io_context);

		/* Łączymy gniazda z odpowiednimi endpointami */
		server_socket.value().connect(server_endpoints.value());

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
	/* Wyciągamy i przetwarzamy wiadomość z bufora. */
	auto message = gui_to_server_buffer.receive_msg_from_gui(
			(size_t) received_length);

	/* Jeżeli wiadomość od gui jest poprawna, przekazujemy dalej do serwera. */
	if (message.has_value()) {
		/* Na podstawie otrzymanej wiadomości, tworzymy nową wiadomość
		 * do serwera, tę wiadomość wstawiamy do bufora. */
		ClientMessageToServer reply = prepare_msg_to_server(message.value());
		return gui_to_server_buffer.insert_msg_to_server(reply);
	}

/* Jeżeli wiadomość od0 GUI przyszła w niepoprawnym formacie, ignorujemy ją. */
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
		}
	} else {
		new_message.type = ClientMessageToServerType::JoinServer;
		new_message.data = parameters.player_name;
	}
	return new_message;
}

void Client::do_receive_from_gui() {
	/* Odbieramy wiadomość od gui i wykonujemy kod w funkcji lambda. */
	gui_socket.value().async_receive(
			boost::asio::buffer(gui_to_server_buffer.get(), BUFFER_SIZE),
			[this](boost::system::error_code ec,
			       std::size_t length) {
				if (!ec) {
					received_length = length;
					do_handle_gui();
				} else {
					/* W przypadku błędu z połączeniem wypisujemy błąd
					 * i kończymy działanie programu. */
					std::cerr << "Error receiving from gui: " << ec.message()
					          << std::endl;
					exit_program(EXIT_FAILURE);
				}
			});
}

void Client::do_handle_gui() {
	/* Obsługujemy wiadomość otrzymaną od GUI i na jej podstawie
	 * zwracamy wiadomość do serwera. Jeżeli wiadomość jest niepoprawna,
	 * ignorujemy ją. Na koniec powracamy do nasłuchiwania */
	auto send = handle_gui_message();
	if (send.has_value()) {
		do_send_server(send.value());
	} else {
		do_receive_from_gui();
	}
}


void Client::do_send_server(size_t send_length) {
	/* Wysyłamy wiadomość do serwera. */
	server_socket.value().async_send(
			boost::asio::buffer(gui_to_server_buffer.get(), send_length),
			[this](boost::system::error_code ec,
			       std::size_t length) {
				if (!ec && length > 0) {
					do_receive_from_gui();
				} else {
					/* W przypadku błędu z połączeniem wypisujemy błąd
					 * i kończymy działanie programu. */
					std::cerr << "Error sending to server: " << ec.message()
					          << std::endl;
					exit_program(EXIT_FAILURE);
				}
			});
}

std::optional<size_t> Client::handle_message_from_server() {
	if (received_length > 0) {
		/* Wyciągamy i przetwarzamy wiadomość z bufora. */
		auto message = server_to_gui_buffer.receive_msg_from_server(
				received_length);
		if (message.has_value()) {
			/* Jeżeli wiadomość jest poprawna, aktualizujemy stan gry. */
			game_info.apply_changes_from_server(message.value());

			/* Na podstawie otrzymanego komunikatu, odpowiadamy GUI:
			 * Turn: wysyłamy Game
			 * AcceptedPlayer, GameEnded, Hello: wysyłamy Lobby
			 * GameStarted: nie wysyłamy nic */
			auto reply = prepare_msg_to_gui(message.value().type);
			if (reply.has_value()) {
				return server_to_gui_buffer.insert_msg_to_display(
						reply.value());
			}
		} else {
			/* Jeżeli wiadomość nie doszła cała, oczekujemy na kolejne pakiety. */
			do_receive_from_server();
		}
	} else if (received_length == 0) {
		/* Jeżeli rozmiar otrzymanej wiadomości wynosi 0, rozłączamy się
		 * i kończymy działanie programu. */
		std::cerr << "Connection suddenly closed.\n";
		exit_program(EXIT_FAILURE);
	} else {
		/* Jeżeli rozmiar otrzymanej wiadomości jest niepoprawny,
		 * rozłączamy się i kończymy działanie programu. */
		std::cerr << "Error: received message from server is not valid.\n";
		exit_program(EXIT_FAILURE);
	}
	return {};
}


std::optional<ClientMessageToDisplay>
Client::prepare_msg_to_gui(ServerMessageToClientType type) {
	/* Tworzymy wiadomość na podstawie aktualnego stanu gry. */
	std::optional<ClientMessageToDisplay> reply;
	/* W przypadku wiadomości GameStarted nie wysyłamy komunikatu do GUI. */
	if (type == ServerMessageToClientType::GameStarted) {
		return {};
		/* W przypadku wiadomości Turn wysyłamy wiadomość typu Gameplay. */
	} else if (type == ServerMessageToClientType::Turn) {
		reply.emplace(GameState::GameplayState,
		              game_info.create_gameplay_msg());
		/* W przypadku innych wiadomości wysyłamy wiadomość typu Lobby. */
	} else {
		reply.emplace(GameState::LobbyState, game_info.create_lobby_msg());
	}
	return reply;
}

void Client::do_receive_from_server() {
//	std::vector<char> temp_buffer;
//	temp_buffer.resize(BUFFER_SIZE, 0);

	/* Odbieramy wiadomość z serwera. */
	server_socket.value().async_receive(
			boost::asio::buffer(server_to_gui_buffer.get(), BUFFER_SIZE),
			[this](boost::system::error_code ec,
			                     std::size_t length) {
				if (!ec) {

					std::cerr << "Otrzymany pakiet:" << std::endl;
					for(size_t i = 0; i < length; i++) {
						std::cerr << (int) server_to_gui_buffer.get()[i] << " | ";
					}
					std::cerr << std::endl;

					received_length = length;
//					server_to_gui_buffer.shift(temp_buffer, length);

					std::cerr <<  "Stan buffera po wpisaniu pakietu: " << std::endl;
					server_to_gui_buffer.print(50);

					do_handle_server();
				} else {
					/* W przypadku błędu z połączeniem wypisujemy błąd
                    * i kończymy działanie programu. */
					std::cerr << "Error receiving from server: " << ec.message()
					          << std::endl;
					exit_program(EXIT_FAILURE);
				}
			});
}

void Client::do_handle_server() {
	/* Obsługujemy otrzymaną od serwera wiadomość. Na jej podstawie
	 * aktualizujemy stan gry oraz wysyłamy wiadomość zwrotną do GUI. */
	auto send = handle_message_from_server();
	if (send.has_value()) {
		do_send_gui(send.value());
	} else {
		do_receive_from_server();
	}

}

void Client::do_send_gui(size_t send_length) {
	/* Wysyłamy wiadomość do GUI. */
	gui_socket.value().async_send_to(
			boost::asio::buffer(server_to_gui_buffer.get(), send_length),
			gui_endpoints.value(),
			[this](boost::system::error_code ec,
			       std::size_t length) {
				if (!ec && length > 0) {
					do_receive_from_server();
				} else {
					/* W przypadku błędu z połączeniem wypisujemy błąd
					* i kończymy działanie programu. */
					std::cerr << "Error sending to GUI: " << ec.message()
					          << std::endl;
					exit_program(EXIT_FAILURE);
				}
			});
}

int main(int argc, char *argv[]) {
	ClientParameters parameters(argc, argv);
	Client client(parameters);
	client.run();
}
