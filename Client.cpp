#include "Client.h"

void Client::connect_display() {
}

void Client::connect_server() {
}

void Client::receive_from_dislay() {
	display_socket.async_receive_from(
			boost::asio::buffer(buffer.get(), BUFFER_SIZE), display_endpoint,
			boost::bind(&Client::handle_message_from_display, this,
			            boost::asio::placeholders::error,
			            boost::asio::placeholders::bytes_transferred));

}

void Client::receive_from_server() {
}

void Client::send_to_display() {
}

void Client::send_to_server() {
}

void Client::initialize() {

}

void Client::run() {
	std::cout << "Client is running" << std::endl;
	while (!finish) {
		sleep(3);
		std::cout << "Nothing to do, client closed" << std::endl;
		finish = true;
	}

}

void Client::handle_message_from_display(const boost::system::error_code& error,
                                         size_t length) {
	auto message = buffer.receive_msg_from_display(length);
	if (message.has_value()) {
		ClientMessageToServer reply = prepare_msg_to_server(message.value());
		buffer.insert_msg_to_server(reply);
	}
}

void Client::handle_message_from_server(const boost::system::error_code& error,
                                        size_t length) {
	auto message = buffer.receive_msg_from_server(length);
	if (message.has_value()) {
		game_info.apply_changes_from_server(message.value());
	}
}

ClientMessageToDisplay Client::prepare_msg_to_display() {
	if (!game_info.is_gameplay()) {
		return {GameState::LobbyState, game_info.create_lobby_msg()};
	}
	return {GameState::GameplayState, game_info.create_gameplay_msg()};
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

