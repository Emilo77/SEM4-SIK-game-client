#include "Client.h"

std::optional<size_t> ServerToGuiHandler::handle_message_from_server() {
	if (received_length > 0) {
		auto message = buffer.receive_msg_from_server((size_t) received_length);
		if (message.has_value()) {
			game_info.apply_changes_from_server(message.value());

			if (should_notify_display(message.value())) {
				ClientMessageToDisplay reply = prepare_msg_to_display();
				return buffer.insert_msg_to_display(reply);
			}
		} else {
			end_program();
		}
	} else {
		end_program();
	}
	return {};
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

void ServerToGuiHandler::receive() {
	received_length = read(server_socket, buffer.get(), BUFFER_SIZE);
	if (received_length < 0) {
		fprintf(stderr,
		        "Error when reading message from server (errno %d,%s)\n",
		        errno, strerror(errno));
		end_program();
	} else if (received_length == 0) {
		end_program();
	}
}

void ServerToGuiHandler::send_to_display(size_t send_length) {
	send_message_to(gui_socket_send, &gui_address_send, buffer.get(),
	                send_length);
}

void ServerToGuiHandler::run() {
	while (!finish) {
		receive();
		auto send = handle_message_from_server();
		if (send.has_value()) {
			send_to_display(send.value());
		}
	}
}
