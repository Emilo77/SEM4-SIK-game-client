#include "Client.h"

void Client::initialize() {}

void Client::run() {}


void Client::handle_message_from_display() {
	std::optional<DisplayMessageToClient> message{};
	buffer.receive_from_display(message);
	if(message.has_value()) {
		gameInfo.apply_changes_from_display(message.value());
	}
}

void Client::handle_message_from_server() {
	std::optional<ServerMessageToClient> message{};
	buffer.receive_from_server(message);
	if(message.has_value()) {
		gameInfo.apply_changes_from_server(message.value());
	}
}


