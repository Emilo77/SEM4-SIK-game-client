#include "Client.h"

std::optional<size_t> GuiToServerHandler::handle_received_message() {
	if (received_length > 0) {
		auto message = buffer.receive_msg_from_display((size_t) received_length);
		if (message.has_value()) {
			ClientMessageToServer reply = prepare_msg_to_server(message.value());
			return buffer.insert_msg_to_server(reply);
		}
	}
	return {};
}

ClientMessageToServer
GuiToServerHandler::prepare_msg_to_server(DisplayMessageToClient &message) {
	ClientMessageToServer new_message;
	if (game_info.is_gameplay()) { //todo może dodać sprawdzanie, że my też gramy
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

void GuiToServerHandler::run() {
	while (!finish) {
		receive();
		auto send = handle_received_message();
		if (send.has_value()) {
			send_to_server(send.value());
		}
		end_program();
		std:: cout << "es działa" << std::endl;
	}
}



