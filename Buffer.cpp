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
	size_t size = sizeof(T);
	number = convert_to_send(number);
	memcpy(buffer + send_index, &number, size);
	send_index += size;
}

void Buffer::insert_raw(const string &str) {
	size_t size = str.size();
	memcpy(buffer + send_index, str.c_str(), size);
	send_index += size;
}

void Buffer::insert(ClientMessageToServerType message) {insert((uint8_t) message);
}

void Buffer::insert(GameState state) { insert((uint8_t) state); }

void Buffer::insert(Direction direction) { insert((uint8_t) direction); }

template<typename T>
void Buffer::receive(T &number) {
	size_t size = sizeof(T);
	memcpy(&number, buffer + read_index, size);
	read_index += size;
	number = convert_to_receive(number);
}

void Buffer::receive_raw(string &str, size_t str_size) {
	str = {buffer + read_index, str_size};
}

void Buffer::insert(Position &position) {
	insert( position.x);
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

void Buffer::receive_event_content(struct BombPlaced &data) {
	receive(data.bomb_id);
	receive(data.position);
}

void Buffer::receive_event_content(struct BombExploded &data) {
	receive(data.bomb_id);
	receive_list(data.robots_destroyed);
	receive_list(data.blocks_destroyed);
}

void Buffer::receive_event_content(struct PlayerMoved &data) {
	receive(data.player_id);
	receive(data.position);
}

void Buffer::receive_event_content(struct BlockPlaced &data) {
	receive(data.position);
}

void Buffer::receive_event(EventType type, Event &event) {
	event.type = type;
	switch (type) {
		case BombPlaced:
			receive_event_content(std::get<struct BombPlaced>(event.data));
			break;
		case BombExploded:
			receive_event_content(std::get<struct BombExploded>(event.data));
			break;
		case PlayerMoved:
			receive_event_content(std::get<struct PlayerMoved>(event.data));
			break;
		case BlockPlaced:
			receive_event_content(std::get<struct BlockPlaced>(event.data));
			break;
	}
}

void Buffer::receive_event_list(std::list<Event> &list) {
	size_t list_size;
	receive(list_size);
	for (uint32_t i = 0; i < list_size; ++i) {
		uint8_t type;
		receive(type);
		std::variant<struct BombPlaced, struct BombExploded,
				struct PlayerMoved, struct BlockPlaced> data;
		Event list_element(EventType::BombPlaced, data);

		receive_event((EventType) type, list_element);

		list.push_back(list_element);
	}
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


size_t Buffer::receive_hello(struct Hello &message) {
	reset_read_index();
	receive(message.server_name);
	receive(message.players_count);
	receive(message.size_x);
	receive(message.size_y);
	receive(message.game_length);
	receive(message.explosion_radius);
	receive(message.bomb_timer);
	return get_read_size();
}
size_t Buffer::receive_accepted_player(struct AcceptedPlayer &message) {
	reset_read_index();
	receive(message.player_id);
	receive(message.player);
	return get_read_size();
}
size_t Buffer::receive_game_started(struct GameStarted &message) {
	receive_map(message.players);
	return get_read_size();
}
size_t Buffer::receive_turn(struct Turn &message) {
	receive(message.turn);
	receive_event_list(message.events);
	return get_read_size();
}
size_t Buffer::receive_game_ended(struct GameEnded &message) {
	receive_map(message.scores);
	return get_read_size();
}

void Buffer::send_lobby(struct Lobby &message) {
	reset_send_index();
	insert(GameState::Lobby);

	insert(message.server_name);
	insert(message.players_count);
	insert(message.size_x);
	insert(message.size_y);
	insert(message.game_length);
	insert(message.explosion_radius);
	insert(message.bomb_timer);
	insert_map(message.players);
}
void Buffer::send_game(struct GamePlay &message) {
	reset_send_index();
	insert(GameState::Gameplay);

	insert(message.server_name);
	insert(message.size_x);
	insert(message.size_y);
	insert(message.game_length);
	insert(message.turn);
	insert_map(message.players);
	insert_map(message.player_positions);
	insert_list(message.blocks);
	insert_list(message.bombs);
}


size_t Buffer::send_to_server(ClientMessageToServer &message) {
	reset_send_index();
	switch (message.type) {
		case ClientMessageToServerType::JoinServer:
			send_join(std::get<string>(message.data));
			break;
		case ClientMessageToServerType::PlaceBombServer:
			send_place_bomb();
			break;
		case ClientMessageToServerType::PlaceBlockServer:
			send_place_block();
			break;
		case ClientMessageToServerType::MoveServer:
			send_move(std::get<Direction>(message.data));
			break;
	}
	return get_send_size();
}
std::optional<ServerMessageToClient> Buffer::receive_from_server(size_t length) {
	auto serverMessage = std::optional<ServerMessageToClient>();
	reset_read_index();
	size_t received = 0;
	uint8_t message;
	receive(message);
	if (invalid_server_message_type(message)) {
		return {};
	}
	switch ((ServerMessageToClientType) message) {
		case Hello:
			received = receive_hello(std::get<struct Hello>(serverMessage->data));
			break;
		case AcceptedPlayer:
			received = receive_accepted_player(std::get<struct AcceptedPlayer>(serverMessage->data));
			break;
		case GameStarted:
			received = receive_game_started(std::get<struct GameStarted>(serverMessage->data));
			break;
		case Turn:
			received = receive_turn(std::get<struct Turn>(serverMessage->data));
			break;
		case GameEnded:
			received = receive_game_ended(std::get<struct GameEnded>(serverMessage->data));
			break;
	}
	if (received != length) {
		return {};
	}
	serverMessage->type = (ServerMessageToClientType) message;
	return serverMessage;
}

size_t Buffer::send_to_display(ClientMessageToDisplay &drawMessage) {
	switch (drawMessage.state) {
		case Lobby:
			send_lobby(std::get<struct Lobby>(drawMessage.data));
			break;
		case Gameplay:
			send_game(std::get<struct GamePlay>(drawMessage.data));
			break;
	}
	return 0;
}
std::optional<DisplayMessageToClient> Buffer::receive_from_display(size_t length) {
	auto message = std::optional<DisplayMessageToClient>();
	reset_read_index();
	uint8_t message_type;
	receive(message_type);
	if (invalid_display_message_type(message_type)) {
		return {};
	}
	if (message_type == DisplayMessageToClientType::MoveDisplay) {
		uint8_t direction;
		receive(direction);
		if (invalid_direction(direction)) {
			return {};
		}
		message->direction = (Direction) direction;
	}
	message->type = (DisplayMessageToClientType) message_type;
	return message;
}


//todo: stworzyć listę eventów i je tam dodawać
//todo: rozwiązać problem z dziedziczeniem po klasie Event
