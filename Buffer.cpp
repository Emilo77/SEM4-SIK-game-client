#include "Buffer.h"

template<typename T>
T Buffer::convert_to_send(T number) {
	switch (sizeof(T)) {
		case 1:
			return number;
		case 2:
			return htobe16(number);
		case 4:
			return htobe32(number);
		default:
			return htobe64(number);
	}
}

template<typename T>
T Buffer::convert_to_receive(T number) {
	switch (sizeof(T)) {
		case 1:
			return number;
		case 2:
			return be16toh(number);
		case 4:
			return be32toh(number);
		default:
			return be64toh(number);
	}
}

template<typename T>
void Buffer::insert(T number) {
	int size = sizeof(T);
	number = convert_to_send(number);
	memcpy(buffer + send_index, &number, size);
	send_index += size;
}

void Buffer::insert_raw(const string &str) {
	size_t size = str.size();
	memcpy(buffer + send_index, str.c_str(), size);
	send_index += size;
}

void Buffer::insert(ClientMessageToServerType message) { insert((uint8_t) message); }

void Buffer::insert(GameState state) { insert((uint8_t) state); }

void Buffer::insert(Direction direction) { insert((uint8_t) direction); }

template<typename T>
void Buffer::receive(T &number) {
	int size = sizeof(T);
	memcpy(&number, buffer + read_index, size);
	read_index += size;
	number = convert_to_receive(number);
}

void Buffer::receive_raw(string &str, size_t str_size) {
	str = {buffer + read_index, str_size};
}

void Buffer::insert(Position &position) {
	insert(position.x);
	insert(position.y);
}

void Buffer::insert(const string &str) {
	insert((uint8_t) str.size());
	insert_raw(str);
}

void Buffer::insert(Player &player) {
	insert(player.name);
	insert(player.address);
}

void Buffer::insert(Bomb &bomb) {
	insert(bomb.position);
	insert(bomb.timer);
}

void Buffer::insert(Event &event) {
}

template<typename T>
//todo ograniczenie na T, że musi być primitive
void Buffer::insert_list(std::list<T> &list) {
	insert((uint32_t) list.size());
	for (T &list_element: list) {
		insert(list_element);
	}
}

template<typename T, typename U>
void Buffer::insert_map(std::map<T, U> &map) {
	insert((uint32_t) map.size());
	for (auto &map_element: map) {
		insert(map_element.first);
		insert(map_element.second);
	}
}

void Buffer::receive(string &str) {
	uint8_t string_size;
	receive(string_size);
	receive_raw(str, string_size);
}

void Buffer::receive(Position &position) {
	receive(position.x);
	receive(position.y);
}

void Buffer::receive(Player &player) {
	receive(player.name);
	receive(player.address);
}

void Buffer::receive(Bomb &bomb) {
	receive(bomb.position);
	receive(bomb.timer);
}

void Buffer::receive(Event &event) {
	//todo
}

template<typename T>
void Buffer::receive_list(std::list<T> &list) {
	uint32_t list_size;
	receive(list_size);
	for (uint32_t i = 0; i < list_size; ++i) {
		T list_element;
		receive(list_element);
		list.push_back(list_element);
	}
}

template<typename T, typename U>
void Buffer::receive_map(std::map<T, U> &map) {
	uint32_t map_size;
	receive(map_size);
	for (uint32_t i = 0; i < map_size; ++i) {
		T map_key;
		U map_value;
		receive(map_key);
		receive(map_value);
		map.insert({map_key, map_value});
	}
}


void Buffer::send_join(string &name) {
	reset_send_index();
	insert(ClientMessageToServerType::JoinServer);
	insert(name);
}

void Buffer::send_place_bomb() {
	reset_send_index();
	insert(ClientMessageToServerType::PlaceBombServer);
}

void Buffer::send_place_block() {
	reset_send_index();
	insert(ClientMessageToServerType::PlaceBlockServer);
}

void Buffer::send_move(Direction direction) {
	reset_send_index();
	insert(ClientMessageToServerType::MoveServer);
	insert(direction);
}


void Buffer::receive_hello(ReceivedServerMessageUnion &message) {
	reset_read_index();
	receive(message.Hello.server_name);
	receive(message.Hello.players_count);
	receive(message.Hello.size_x);
	receive(message.Hello.size_y);
	receive(message.Hello.game_length);
	receive(message.Hello.explosion_radius);
	receive(message.Hello.bomb_timer);
}
void Buffer::receive_accepted_player(ReceivedServerMessageUnion &message) {
	reset_read_index();
	receive(message.AcceptedPlayer.player_id);
	receive(message.AcceptedPlayer.player);
}
void Buffer::receive_game_started(ReceivedServerMessageUnion &message) {
	receive_map(message.GameStarted.players);
}
void Buffer::receive_turn(ReceivedServerMessageUnion &message) {
	receive(message.Turn.turn);
	receive_list(message.Turn.events);
}
void Buffer::receive_game_ended(ReceivedServerMessageUnion &message) {
	receive_map(message.GameEnded.scores);
}

void Buffer::send_lobby(DrawMessageUnion &message) {
	reset_send_index();
	insert(GameState::Lobby);

	insert(message.Lobby.server_name);
	insert(message.Lobby.players_count);
	insert(message.Lobby.size_x);
	insert(message.Lobby.size_y);
	insert(message.Lobby.game_length);
	insert(message.Lobby.explosion_radius);
	insert(message.Lobby.bomb_timer);
	insert_map(message.Lobby.players);
}
void Buffer::send_game(DrawMessageUnion &message) {
	reset_send_index();
	insert(GameState::Gameplay);

	insert(message.Gameplay.server_name);
	insert(message.Gameplay.size_x);
	insert(message.Gameplay.size_y);
	insert(message.Gameplay.game_length);
	insert(message.Gameplay.turn);
	insert_map(message.Gameplay.players);
	insert_map(message.Gameplay.player_positions);
	insert_list(message.Gameplay.blocks);
	insert_list(message.Gameplay.bombs);
}


size_t Buffer::send_to_server(ClientMessageToServerType clientMessage, std::string name, Direction direction) {
	reset_send_index();
	switch (clientMessage) {
		case ClientMessageToServerType::JoinServer:
			send_join(name);
			break;
		case ClientMessageToServerType::PlaceBombServer:
			send_place_bomb();
			break;
		case ClientMessageToServerType::PlaceBlockServer:
			send_place_block();
			break;
		case ClientMessageToServerType::MoveServer:
			send_move(direction);
			break;
	}
	return get_size();
}
void Buffer::receive_from_server(std::optional<ServerMessageToClient> &serverMessage) {
	reset_read_index();
	uint8_t message;
	receive(message);
	serverMessage->type = (ServerMessageToClientType) message;
	switch (message) {
		case Hello:
			receive_hello(serverMessage->data);
			break;
		case AcceptedPlayer:
			receive_accepted_player(serverMessage->data);
			break;
		case GameStarted:
			receive_game_started(serverMessage->data);
			break;
		case Turn:
			receive_turn(serverMessage->data);
			break;
		case GameEnded:
			receive_game_ended(serverMessage->data);
			break;
	}
}

size_t Buffer::send_to_display(ClientMessageToDisplay &drawMessage) {
	switch (drawMessage.state) {
		case Lobby:
			send_lobby(drawMessage.data);
			break;
		case Gameplay:
			send_game(drawMessage.data);
			break;
	}
	return 0;
}
void Buffer::receive_from_display(std::optional<DisplayMessageToClient> &message) {
	reset_read_index();
	uint8_t message_type;
	receive(message_type);
	if (message_type > MAX_DISPLAY_INPUT_TYPE) {
		return;
	}
	if (message_type == DisplayMessageToClientType::MoveDisplay) {
		uint8_t direction;
		receive(direction);
		if (direction > MAX_DIRECTION_TYPE) {
			return;
		}
		message->direction = (Direction) direction;
	}
	message->type = (DisplayMessageToClientType) message_type;

}


//todo: stworzyć listę eventów i je tam dodawać
//todo: rozwiązać problem z dziedziczeniem po klasie Event
