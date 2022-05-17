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
	auto message = buffer.receive_from_display(length);
	if (message.has_value()) {

	}
}

void Client::handle_message_from_server(size_t length) {
	auto message = buffer.receive_from_server(length);
	if (message.has_value()) {
		game_info.apply_changes_from_server(message.value());
	}
}

size_t Client::put_server_msg_to_buffer(ClientMessageToServer &message) {
	buffer.reset_send_index();
	switch (message.type) {
		case ClientMessageToServerType::JoinServer:
			buffer.send_join(std::get<string>(message.data));
			break;
		case ClientMessageToServerType::PlaceBombServer:
			buffer.send_place_bomb();
			break;
		case ClientMessageToServerType::PlaceBlockServer:
			buffer.send_place_block();
			break;
		case ClientMessageToServerType::MoveServer:
			buffer.send_move(std::get<Direction>(message.data));
			break;
	}
	return buffer.get_send_size();
}

std::optional<ServerMessageToClient>
Client::get_server_msg_from_buffer(size_t length) {
	auto serverMessage = std::optional<ServerMessageToClient>();
	buffer.reset_read_index();
	size_t received = 0;
	uint8_t message;
	buffer.receive(message);
	if (invalid_server_message_type(message)) {
		return {};
	}
	switch ((ServerMessageToClientType) message) {
		case Hello:
			received = buffer.receive_hello(
					std::get<struct Hello>(serverMessage->data));
			break;
		case AcceptedPlayer:
			received = buffer.receive_accepted_player(
					std::get<struct AcceptedPlayer>(serverMessage->data));
			break;
		case GameStarted:
			received = buffer.receive_game_started(
					std::get<struct GameStarted>(serverMessage->data));
			break;
		case Turn:
			received = buffer.receive_turn(
					std::get<struct Turn>(serverMessage->data));
			break;
		case GameEnded:
			received = buffer.receive_game_ended(
					std::get<struct GameEnded>(serverMessage->data));
			break;
	}
	if (received != length) {
		return {};
	}
	serverMessage->type = (ServerMessageToClientType) message;
	return serverMessage;
}

size_t Client::put_display_msg_to_buffer(ClientMessageToDisplay &drawMessage) {
	switch (drawMessage.state) {
		case Lobby:
			buffer.send_lobby(std::get<struct Lobby>(drawMessage.data));
			break;
		case Gameplay:
			buffer.send_game(std::get<struct GamePlay>(drawMessage.data));
			break;
	}
	return 0;
}

std::optional<DisplayMessageToClient>
Client::get_display_msg_from_buffer(size_t length) {
	auto message = std::optional<DisplayMessageToClient>();
	buffer.reset_read_index();
	uint8_t message_type;
	buffer.receive(message_type);
	if (invalid_display_message_type(message_type)) {
		return {};
	}
	if (message_type == DisplayMessageToClientType::MoveDisplay) {
		uint8_t direction;
		buffer.receive(direction);
		if (invalid_direction(direction)) {
			return {};
		}
		message->direction = (Direction) direction;
	}
	message->type = (DisplayMessageToClientType) message_type;
	return message;
}

ClientMessageToDisplay Client::prepare_msg_to_display() {
	ClientMessageToDisplay new_message;
	if (!game_info.is_gameplay()) {
		new_message.state = GameState::Lobby;
		new_message.data = game_info.create_lobby_msg();
	} else {
		new_message.state = GameState::Gameplay;
		new_message.data = game_info.create_gameplay_msg();
	}

	return new_message;
}

ClientMessageToServer Client::prepare_msg_to_server(DisplayMessageToClient &message) {
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

