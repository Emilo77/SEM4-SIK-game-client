#include "Buffer.h"

static bool invalid_direction(uint8_t direction) {return direction > 3;}

static bool invalid_server_message_type(uint8_t type) {return type > 4;}

static bool invalid_display_message_type(uint8_t type) {return type > 2;}

uint8_t Buffer::convert_to_send(uint8_t number) { return number; }

uint16_t Buffer::convert_to_send(uint16_t number) { return htobe16(number); }

uint32_t Buffer::convert_to_send(uint32_t number) { return htobe32(number); }

uint8_t Buffer::convert_to_receive(uint8_t number) { return number; }

uint16_t Buffer::convert_to_receive(uint16_t number) { return be16toh(number); }

uint32_t Buffer::convert_to_receive(uint32_t number) { return be32toh(number); }

void Buffer::insert_raw(const string &str) {
	size_t size = str.size();
	memcpy(buffer + send_index, str.c_str(), size);
	send_index += size;
}

void Buffer::insert(uint8_t number) {
	number = convert_to_send(number);
	memcpy(buffer + send_index, &number, sizeof(number));
	send_index += sizeof(number);
}

void Buffer::insert(uint16_t number) {
	number = convert_to_send(number);
	memcpy(buffer + send_index, &number, sizeof(number));
	send_index += sizeof(number);
}

void Buffer::insert(uint32_t number) {
	number = convert_to_send(number);
	memcpy(buffer + send_index, &number, sizeof(number));
	send_index += sizeof(number);
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

void Buffer::insert_list_positions(std::list<Position> &positions) {
	insert((uint32_t) positions.size());
	for (Position &list_element: positions) {
		insert(list_element);
	}
}

void Buffer::insert_list_bombs(std::list<Bomb> &bombs) {
	insert((uint32_t) bombs.size());
	for (Bomb &list_element: bombs) {
		insert(list_element);
	}
}

void Buffer::insert_map_players(std::map<player_id_t, Player> &players) {
	insert((uint32_t) players.size());
	for (auto &map_element: players) {
		insert(map_element.first);
		insert(map_element.second);
	}
}

void Buffer::insert_map_scores(std::map<player_id_t, score_t> &scores) {
	insert((uint32_t) scores.size());
	for (auto &map_element: scores) {
		insert(map_element.first);
		insert(map_element.second);
	}
}

void Buffer::insert_map_positions(std::map<player_id_t, Position> &positions) {
	insert((uint32_t) positions.size());
	for (auto &map_element: positions) {
		insert(map_element.first);
		insert(map_element.second);
	}
}

void Buffer::receive_raw(string &str, size_t str_size) {
	str = {buffer + read_index, str_size};
	read_index += str_size;
}

void Buffer::receive(uint8_t &number) {
	size_t size = sizeof(number);
	memcpy(&number, buffer + read_index, size);
	read_index += size;
	number = convert_to_receive(number);
}

void Buffer::receive(uint16_t &number) {
	size_t size = sizeof(number);
	memcpy(&number, buffer + read_index, size);
	read_index += size;
	number = convert_to_receive(number);
}

void Buffer::receive(uint32_t &number) {
	size_t size = sizeof(number);
	memcpy(&number, buffer + read_index, size);
	read_index += size;
	number = convert_to_receive(number);
}

void Buffer::receive(string &str) {
	uint8_t string_size = 0;
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

void Buffer::receive_event_content(struct BombPlaced &data) {
	receive(data.bomb_id);
	receive(data.position);
}

void Buffer::receive_event_content(struct BombExploded &data) {
	receive(data.bomb_id);
	receive_list_player_ids(data.robots_destroyed);
	receive_list_positions(data.blocks_destroyed);
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

void Buffer::receive_list_events(std::vector<Event> &vector) {
	uint32_t vector_size;
	receive(vector_size);
	for (uint32_t i = 0; i < vector_size; ++i) {
		uint8_t type;
		receive(type);
		std::variant<struct BombPlaced, struct BombExploded,
				struct PlayerMoved, struct BlockPlaced> data;
		Event event((EventType) type, data);

		receive_event((EventType) type, event);

		vector.push_back(event);
	}
}

void Buffer::receive_list_player_ids(std::vector<player_id_t> &ids) {
	uint32_t vector_size;
	receive(vector_size);
	for (uint32_t i = 0; i < vector_size; ++i) {
		player_id_t id;
		receive(id);
		ids.push_back(id);
	}
}

void Buffer::receive_list_positions(std::vector<Position> &positions) {
	uint32_t vector_size;
	receive(vector_size);
	for (uint32_t i = 0; i < vector_size; ++i) {
		Position position;
		receive(position);
		positions.push_back(position);
	}
}

void Buffer::receive_map_players(std::map<player_id_t, Player> &players) {
	uint32_t map_size;
	receive(map_size);
	for (uint32_t i = 0; i < map_size; ++i) {
		player_id_t id;
		Player player;
		receive(id);
		receive(player);
		players.insert({id, player});
	}
}

void Buffer::receive_map_scores(std::map<player_id_t, score_t> &scores) {
	uint32_t map_size;
	receive(map_size);
	for (uint32_t i = 0; i < map_size; ++i) {
		player_id_t id;
		score_t score;
		receive(id);
		receive(score);
		scores.insert({id, score});
	}
}

void Buffer::insert_join(string &name) {
	insert((uint8_t) ClientMessageToServerType::JoinServer);
	insert(name);
}

void Buffer::insert_place_bomb() {
	insert((uint8_t) ClientMessageToServerType::PlaceBombServer);
}

void Buffer::insert_place_block() {
	insert((uint8_t) ClientMessageToServerType::PlaceBlockServer);
}

void Buffer::insert_move(Direction direction) {
	insert((uint8_t) ClientMessageToServerType::MoveServer);
	insert((uint8_t) direction);
}

void Buffer::receive_hello(struct Hello &message) {
	receive(message.server_name);
	receive(message.players_count);
	receive(message.size_x);
	receive(message.size_y);
	receive(message.game_length);
	receive(message.explosion_radius);
	receive(message.bomb_timer);

}

void Buffer::receive_accepted_player(struct AcceptedPlayer &message) {
	receive(message.player_id);
	receive(message.player);

}

void Buffer::receive_game_started(struct GameStarted &message) {
	receive_map_players(message.players);

}

void Buffer::receive_turn(struct Turn &message) {
	receive(message.turn);
	receive_list_events(message.events);

}

void Buffer::receive_game_ended(struct GameEnded &message) {
	receive_map_scores(message.scores);

}

void Buffer::send_lobby(Lobby &message) {
	insert((uint8_t) GameState::LobbyState);
	insert(message.server_name);
	insert(message.players_count);
	insert(message.size_x);
	insert(message.size_y);
	insert(message.game_length);
	insert(message.explosion_radius);
	insert(message.bomb_timer);
	insert_map_players(message.players);
}

void Buffer::send_game(GamePlay &message) {
	insert((uint8_t) GameState::GameplayState);
	insert(message.server_name);
	insert(message.size_x);
	insert(message.size_y);
	insert(message.game_length);
	insert(message.turn);
	insert_map_players(message.players);
	insert_map_positions(message.player_positions);
	insert_list_positions(message.blocks);
	insert_list_bombs(message.bombs);
	insert_list_positions(message.explosions);
	insert_map_scores(message.scores);
}


size_t Buffer::insert_msg_to_server(ClientMessageToServer &message) {
	reset_send_index();
	switch (message.type) {
		case ClientMessageToServerType::JoinServer:
			insert_join(std::get<string>(message.data));
			break;
		case ClientMessageToServerType::PlaceBombServer:
			insert_place_bomb();
			break;
		case ClientMessageToServerType::PlaceBlockServer:
			insert_place_block();
			break;
		case ClientMessageToServerType::MoveServer:
			insert_move(std::get<Direction>(message.data));
			break;
	}
	return get_send_size();
}

std::optional<ServerMessageToClient>
Buffer::receive_msg_from_server(size_t expected_size) {
	auto serverMessage = std::optional<ServerMessageToClient>();
	reset_read_index();
	uint8_t message;
	receive(message);
	if (invalid_server_message_type(message)) {
		return {};
	}
	switch ((ServerMessageToClientType) message) {
		case Hello:
			receive_hello(std::get<struct Hello>(serverMessage->data));
			break;
		case AcceptedPlayer:
			receive_accepted_player(
					std::get<struct AcceptedPlayer>(serverMessage->data));
			break;
		case GameStarted:
			receive_game_started(
					std::get<struct GameStarted>(serverMessage->data));
			break;
		case Turn:
			receive_turn(std::get<struct Turn>(serverMessage->data));
			break;
		case GameEnded:
			receive_game_ended(std::get<struct GameEnded>(serverMessage->data));
			break;
	}
	if (get_read_size() != expected_size) {
		return {}; //może niepotrzebne, wtedy zakomentowaC
	}
	serverMessage->type = (ServerMessageToClientType) message;
	return serverMessage;
}

size_t Buffer::insert_msg_to_display(ClientMessageToDisplay &drawMessage) {
	reset_send_index();
	switch (drawMessage.state) {
		case LobbyState:
			send_lobby(std::get<Lobby>(drawMessage.data));
			break;
		case GameplayState:
			send_game(std::get<GamePlay>(drawMessage.data));
			break;
	}
	return 0;
}

std::optional<DisplayMessageToClient>
Buffer::receive_msg_from_display(size_t expected_size) {
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

	if (get_read_size() != expected_size) {
		return {};
	}
	return message;
}


