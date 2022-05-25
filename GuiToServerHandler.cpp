#include "Handlers.h"


std::optional<size_t> GuiToServerHandler::hangle_gui_message() {
	if (received_length > 0) {
		auto message = buffer.receive_msg_from_display(
				(size_t) received_length);
		if (message.has_value()) {
			ClientMessageToServer reply = prepare_msg_to_server(
					message.value());
			return buffer.insert_msg_to_server(reply);
		}
	}
//	end_program();
	return {};
}

ClientMessageToServer
GuiToServerHandler::prepare_msg_to_server(DisplayMessageToClient &message) {
	ClientMessageToServer new_message;

	bool is_gameplay = game_info.is_gameplay();

	if (is_gameplay) { //todo może dodać sprawdzanie, że my też gramy
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

void GuiToServerHandler::do_receive() {
	gui_socket.async_receive(boost::asio::buffer(buffer.get(), BUFFER_SIZE),
	                         [this](boost::system::error_code ec,
	                                std::size_t length) {
		                         received_length = length;
		                         do_handle(ec);
	                         });
}

void GuiToServerHandler::do_handle(const boost::system::error_code &ec) {
	if(!ec) {
		auto send = hangle_gui_message();
		if (send.has_value()) {
			do_send(send.value());
		}
	} else {
		std:: cout << "Error: " << ec.message() << std::endl;
		exit(1);
	}
}


void GuiToServerHandler::do_send(size_t send_length) {
	server_socket.async_send(boost::asio::buffer(buffer.get(), send_length),
	                         [this](boost::system::error_code ec,
	                                std::size_t length) {
		                         if (!ec) {
			                         do_receive();
		                         } else {
//									 end_program();
		                         }
	                         });
}



