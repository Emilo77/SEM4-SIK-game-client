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
	while(!finish) {

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
		gameInfo.apply_changes_from_server(message.value());
	}
}
