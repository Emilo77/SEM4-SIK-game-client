#include "Buffer.h"

static void check_direction(uint8_t direction) {
	if (direction > Direction::Left) {
		throw InvalidMessage();
	}
}

static void check_gui_message_type(uint8_t type) {
	if (type > GuiMessageToClientType::MoveGui) {
		throw InvalidMessage();
	}
}

static void check_server_message_type(uint8_t type) {
	if (type > ServerMessageToClientType::GameEnded) {
		throw InvalidMessage();
	}
}

void Buffer::check_if_message_incomplete(size_t variable) const {
	if (read_index + variable > end_of_data_index) {
		throw IncompleteMessage();
	}
}

uint16_t Buffer::convert_to_send(uint16_t number) { return htobe16(number); }

uint32_t Buffer::convert_to_send(uint32_t number) { return htobe32(number); }

uint16_t Buffer::convert_to_receive(uint16_t number) { return be16toh(number); }

uint32_t Buffer::convert_to_receive(uint32_t number) { return be32toh(number); }

void Buffer::insert_raw(const string &str) {
	size_t size = str.size();
	memcpy(&send_buffer[send_index], str.c_str(), size);
	send_index += size;
}

void Buffer::insert(uint8_t number) {
	memcpy(&send_buffer[send_index], &number, sizeof(number));
	send_index += sizeof(number);
}

void Buffer::insert(uint16_t number) {
	number = convert_to_send(number);
	memcpy(&send_buffer[send_index], &number, sizeof(number));
	send_index += sizeof(number);
}

void Buffer::insert(uint32_t number) {
	number = convert_to_send(number);
	memcpy(&send_buffer[send_index], &number, sizeof(number));
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
	try {
		check_if_message_incomplete(str_size);
		str = {&receive_buffer[read_index], str_size};
		read_index += str_size;

	} catch (IncompleteMessage &e) {
		throw e;
	}
}

void Buffer::receive(uint8_t &number) {
	try {
		size_t size = sizeof(number);
		check_if_message_incomplete(size);
		memcpy(&number, &receive_buffer[read_index], size);
		read_index += size;

	} catch (IncompleteMessage &e) {
		throw e;
	}
}

void Buffer::receive(uint16_t &number) {
	try {
		size_t size = sizeof(number);
		check_if_message_incomplete(size);
		memcpy(&number, &receive_buffer[read_index], size);
		read_index += size;
		number = convert_to_receive(number);

	} catch (IncompleteMessage &e) {
		throw e;
	}
}

void Buffer::receive(uint32_t &number) {
	try {
		size_t size = sizeof(number);
		check_if_message_incomplete(size);
		memcpy(&number, &receive_buffer[read_index], size);
		read_index += size;
		number = convert_to_receive(number);

	} catch (IncompleteMessage &e) {
		throw e;
	}
}

void Buffer::receive(string &str) {
	try {
		uint8_t string_size = 0;
		receive(string_size);
		receive_raw(str, string_size);

	} catch (IncompleteMessage &e) {
		throw e;
	}
}

void Buffer::receive(Position &position) {
	try {
		receive(position.x);
		receive(position.y);

	} catch (IncompleteMessage &e) {
		throw e;
	}
}

void Buffer::receive(Player &player) {
	try {
		receive(player.name);
		receive(player.address);

	} catch (IncompleteMessage &e) {
		throw e;
	}
}

struct BombPlaced Buffer::receive_bomb_placed() {
	try {
		struct BombPlaced event_content;
		receive(event_content.bomb_id);
		receive(event_content.position);
		return event_content;

	} catch (IncompleteMessage &e) {
		throw e;
	}
}

struct BombExploded Buffer::receive_bomb_exploded() {
	try {
		struct BombExploded event_content;
		receive(event_content.bomb_id);
		receive_list_player_ids(event_content.robots_destroyed);
		receive_list_positions(event_content.blocks_destroyed);
		return event_content;

	} catch (IncompleteMessage &e) {
		throw e;
	}

}

struct PlayerMoved Buffer::receive_player_moved() {
	try {
		struct PlayerMoved event_content;
		receive(event_content.player_id);
		receive(event_content.position);
		return event_content;

	} catch (IncompleteMessage &e) {
		throw e;
	}
}

struct BlockPlaced Buffer::receive_block_placed() {
	try {
		struct BlockPlaced event_content;
		receive(event_content.position);
		return event_content;

	} catch (IncompleteMessage &e) {
		throw e;
	}

}

Event Buffer::receive_event() {
	try {
		uint8_t type;
		receive(type);
		std::variant<struct BombPlaced, struct BombExploded,
				struct PlayerMoved, struct BlockPlaced> data;
		switch ((EventType) type) {
			case BombPlaced:
				data = receive_bomb_placed();
				break;
			case BombExploded:
				data = receive_bomb_exploded();
				break;
			case PlayerMoved:
				data = receive_player_moved();
				break;
			case BlockPlaced:
				data = receive_block_placed();
				break;
		}
		return Event((EventType) type, data);

	} catch (IncompleteMessage &e) {
		throw e;
	}
}

void Buffer::receive_list_events(std::vector<Event> &vector) {
	try {
		uint32_t vector_size;
		receive(vector_size);
		for (uint32_t i = 0; i < vector_size; ++i) {
			vector.push_back(receive_event());
		}
	} catch (IncompleteMessage &e) {
		throw e;
	}
}

void Buffer::receive_list_player_ids(std::vector<player_id_t> &ids) {
	try {
		uint32_t vector_size;
		receive(vector_size);
		for (uint32_t i = 0; i < vector_size; ++i) {
			player_id_t id;
			receive(id);
			ids.push_back(id);
		}
	} catch (IncompleteMessage &e) {
		throw e;
	}
}

void Buffer::receive_list_positions(std::vector<Position> &positions) {
	try {
		uint32_t vector_size;
		receive(vector_size);
		for (uint32_t i = 0; i < vector_size; ++i) {
			Position position;
			receive(position);
			positions.push_back(position);
		}
	} catch (IncompleteMessage &e) {
		throw e;
	}
}

void Buffer::receive_map_players(std::map<player_id_t, Player> &players) {
	try {
		uint32_t map_size;
		receive(map_size);
		for (uint32_t i = 0; i < map_size; ++i) {
			player_id_t id;
			Player player;
			receive(id);
			receive(player);
			players.insert_or_assign(id, player);
		}
	} catch (IncompleteMessage &e) {
		throw e;
	}
}

void Buffer::receive_map_scores(std::map<player_id_t, score_t> &scores) {
	try {
		uint32_t map_size;
		receive(map_size);
		for (uint32_t i = 0; i < map_size; ++i) {
			player_id_t id;
			score_t score;
			receive(id);
			receive(score);
			scores.insert_or_assign(id, score);
		}
	} catch (IncompleteMessage &e) {
		throw e;
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

struct Hello Buffer::receive_hello() {
	try {
		struct Hello hello;
		receive(hello.server_name);
		receive(hello.players_count);
		receive(hello.size_x);
		receive(hello.size_y);
		receive(hello.game_length);
		receive(hello.explosion_radius);
		receive(hello.bomb_timer);
		return hello;

	} catch (IncompleteMessage &e) {
		throw e;
	}
}

struct AcceptedPlayer Buffer::receive_accepted_player() {
	try {
		struct AcceptedPlayer accepted_player;
		receive(accepted_player.player_id);
		receive(accepted_player.player);
		return accepted_player;

	} catch (IncompleteMessage &e) {
		throw e;
	}
}

struct GameStarted Buffer::receive_game_started() {
	try {
		struct GameStarted game_started;
		receive_map_players(game_started.players);
		return game_started;

	} catch (IncompleteMessage &e) {
		throw e;
	}
}

struct Turn Buffer::receive_turn() {
	try {
		struct Turn turn;
		receive(turn.turn_number);
		receive_list_events(turn.events);
		return turn;

	} catch (IncompleteMessage &e) {
		throw e;
	}
}

struct GameEnded Buffer::receive_game_ended() {
	try {
		struct GameEnded game_ended;
		receive_map_scores(game_ended.scores);
		return game_ended;

	} catch (IncompleteMessage &e) {
		throw e;
	}
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

void Buffer::initialize() {
	receive_buffer.resize(BUFFER_SIZE, 0);
	send_buffer.resize(BUFFER_SIZE, 0);
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
Buffer::receive_msg_from_server(size_t received_size) {
	end_of_data_index += received_size;
	reset_read_index();
	auto serverMessage = std::optional<ServerMessageToClient>();
	uint8_t message;
	std::variant<struct Hello, struct AcceptedPlayer,
			struct GameStarted, struct Turn, struct GameEnded> data;
	try {
		receive(message);
		check_server_message_type(message);

		switch ((ServerMessageToClientType) message) {
			case Hello:
				data = receive_hello();
				serverMessage.emplace(Hello, data);
				break;
			case AcceptedPlayer:
				data = receive_accepted_player();
				serverMessage.emplace(AcceptedPlayer, data);
				break;
			case GameStarted:
				data = receive_game_started();
				serverMessage.emplace(GameStarted, data);
				break;
			case Turn:
				data = receive_turn();
				serverMessage.emplace(Turn, data);
				break;
			case GameEnded:
				data = receive_game_ended();
				serverMessage.emplace(GameEnded, data);
				break;
		}

	} catch (IncompleteMessage &e) {
		set_shift(end_of_data_index);
		throw e;
	}

	end_of_data_index -= get_read_size();
	set_shift(end_of_data_index);

	for (size_t i = 0; i < end_of_data_index; i++) {
		receive_buffer[i] = receive_buffer[i + get_read_size()];
	}

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
	return get_send_size();
}

GuiMessageToClient Buffer::receive_msg_from_gui(size_t received_size) {
	end_of_data_index = BUFFER_SIZE;
	reset_read_index();

	auto message = std::optional<GuiMessageToClient>();
	uint8_t message_type;
	uint8_t direction;

	try {
		receive(message_type);
		check_gui_message_type(message_type);
		if (message_type == GuiMessageToClientType::MoveGui) {
			receive(direction);
			check_direction(direction);

			return {(GuiMessageToClientType) message_type,
			                          static_cast<Direction>(direction)};
		}

		if (get_read_size() != received_size) {
			throw InvalidMessage();
		}

		std:: cerr << "Poprawna wiadomość GUI" << std::endl;
		return GuiMessageToClient((GuiMessageToClientType) message_type);

	} catch (InvalidMessage &e) {
		std:: cerr << "Złapało wyjątek " << std::endl;
		throw e;
	} catch (IncompleteMessage &e) {
		throw InvalidMessage();
	}
}

