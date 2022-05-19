#include "Client.h"

void connect_with_server() {

}

void receive_from_server() {

}

void ServerToGuiHandler::handle_message_from_server(
		const boost::system::error_code &error,
		size_t length) {
	if (!error) {
		auto message = buffer.receive_msg_from_server(length);
		if (message.has_value()) {
			game_info.apply_changes_from_server(message.value());

			if (should_notify_display(message.value())) {
				ClientMessageToDisplay reply = prepare_msg_to_display();
				buffer.insert_msg_to_display(reply);
			}
		} else {
			// rozłącz z serwerem
		}
	} else {
		// rozłącz z serwerem (?)
	}
}


ClientMessageToDisplay ServerToGuiHandler::prepare_msg_to_display() {
	if (!game_info.is_gameplay()) {
		return {GameState::LobbyState, game_info.create_lobby_msg()};
	}
	return {GameState::GameplayState, game_info.create_gameplay_msg()};
}

void send_to_display() {

}

bool ServerToGuiHandler::should_notify_display(ServerMessageToClient &message) {
	return (message.type == AcceptedPlayer && !game_info.is_gameplay())
	       || (message.type == Turn && game_info.is_gameplay());
}