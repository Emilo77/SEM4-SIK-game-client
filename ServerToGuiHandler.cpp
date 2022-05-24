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
//			end_program();
		}
	} else {
//		end_program();
	}
	return {};
}


ClientMessageToDisplay ServerToGuiHandler::prepare_msg_to_display() {
	if (!game_info.is_gameplay()) {
		return {GameState::LobbyState, game_info.create_lobby_msg()};
	}
	return {GameState::GameplayState, game_info.create_gameplay_msg()};
}

bool ServerToGuiHandler::should_notify_display(ServerMessageToClient &message) {
	return (message.type == AcceptedPlayer && !game_info.is_gameplay())
	       || (message.type == Turn && game_info.is_gameplay());
}

void ServerToGuiHandler::do_receive() {
	server_socket.async_receive(boost::asio::buffer(buffer.get(), BUFFER_SIZE),
	                            [this](boost::system::error_code ec, std::size_t length) {
		                            received_length = length;
		                            do_handle(ec);
	                            });
}

void ServerToGuiHandler::do_handle(const boost::system::error_code &ec) {
	if (!ec) {
		auto send = handle_message_from_server();
		if (send.has_value()) {
			do_send(send.value());
		}
	} else {
//		end_program();
	}
}

void ServerToGuiHandler::do_send(size_t send_length) {
	gui_socket.async_send_to(boost::asio::buffer(buffer.get(), send_length), gui_endpoint,
	                         [this](boost::system::error_code ec, std::size_t length) {
		                         if (!ec) {
			                         do_receive();

		                         } else {
			                         gui_socket.close();
			                         std::cerr << "Error: " << ec.message() << std::endl;
		                         }
	                         });
}