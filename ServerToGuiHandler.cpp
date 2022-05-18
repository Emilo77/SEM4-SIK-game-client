#include "Client.h"

void connect_with_server() {

}

void receive_from_server() {

}

void ServerToGuiHandler::handle_message_from_server(const boost::system::error_code &error,
                                        size_t length) {
	auto message = buffer.receive_msg_from_server(length);
	if (message.has_value()) {
		game_info.apply_changes_from_server(message.value());
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