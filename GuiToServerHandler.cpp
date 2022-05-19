#include "Client.h"

void GuiToServerHandler::connect_with_server() {
	boost::asio::async_connect(socket, server_endpoint,
	                           boost::bind(
			                           &GuiToServerHandler::handle_connect,
			                           this,
			                           boost::asio::placeholders::error));
}


void
GuiToServerHandler::handle_connect(const boost::system::error_code &error) {
	if (!error) {
		boost::asio::async_read(socket,
		                        boost::asio::buffer(buffer.get(),
		                                            BUFFER_SIZE),
		                        boost::bind(
				                        &GuiToServerHandler::handle_message_from_display,
				                        this,
				                        boost::asio::placeholders::error,
				                        boost::asio::placeholders::bytes_transferred));
	} else {
		//todo
	}
}

void GuiToServerHandler::handle_message_from_display(
		const boost::system::error_code &error,
		size_t length) {
	if (!error && length > 0) {
		auto message = buffer.receive_msg_from_display(length);
		if (message.has_value()) {
			ClientMessageToServer reply = prepare_msg_to_server(
					message.value());
			buffer.insert_msg_to_server(reply);
		}
	}
}

ClientMessageToServer
GuiToServerHandler::prepare_msg_to_server(DisplayMessageToClient &message) {
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


