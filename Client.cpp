#include "Client.h"

void Client::initialize() {}

void Client::run() {}


DisplayMessageToClient Client::handle_message_from_display() {
	std::optional<DisplayMessageToClient> message{};
	buffer.receive_from_display(message);

}

ServerMessageToClient Client::handle_message_from_server() {
}
