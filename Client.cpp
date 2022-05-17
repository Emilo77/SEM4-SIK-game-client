#include "Client.h"

void Client::bind_sockets() {
}

void Client::receive_from_dislay() {
}

void Client::receive_from_server() {
}

void Client::send_to_display() {
}

void Client::send_to_server() {
}

void Client::initialize() {
	bind_sockets();
}

void Client::run() {
	std::cout << "Client is running" << std::endl;
	while (!finish) {

	}

}

void Client::handle_message_from_display(size_t length) {
	auto message = buffer.receive_msg_from_display(length);
	if (message.has_value()) {

	}
}

void Client::handle_message_from_server(size_t length) {
	auto message = buffer.receive_msg_from_server(length);
	if (message.has_value()) {
		game_info.apply_changes_from_server(message.value());
	}
}

ClientMessageToDisplay Client::prepare_msg_to_display() {
	if (!game_info.is_gameplay()) {
		return {GameState::Lobby, game_info.create_lobby_msg()};
	}
		return {GameState::Gameplay, game_info.create_gameplay_msg()};
}

ClientMessageToServer
Client::prepare_msg_to_server(DisplayMessageToClient &message) {
	ClientMessageToServer new_message;
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

