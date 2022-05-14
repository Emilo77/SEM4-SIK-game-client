#include "Client.h"

void Client::initialize() {}

void Client::run() {}


void Client::handle_message_from_display() {
	auto  message = buffer.receive_from_display();
	if(message.has_value()) {
		gameInfo.apply_changes_from_display(message.value());
	}
}

void Client::handle_message_from_server() {
	auto message = buffer.receive_from_server();
	if(message.has_value()) {
		gameInfo.apply_changes_from_server(message.value());
	}
}


