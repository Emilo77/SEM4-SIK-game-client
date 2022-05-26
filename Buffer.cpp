#include "Buffer.h"

static bool invalid_direction(uint8_t direction) { return direction > 3; }

static bool invalid_server_message_type(uint8_t type) { return type > 4; }

static bool invalid_gui_message_type(uint8_t type) { return type > 2; }

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

struct BombPlaced Buffer::receive_bomb_placed() {
	struct BombPlaced event_content;
	receive(event_content.bomb_id);
	std::cerr << "received bomb_id: " << event_content.bomb_id << std::endl;
	receive(event_content.position);
	std::cerr << "received : bomb position" << event_content.position.x << " " << event_content.position.y << std::endl;
	return event_content;
}

struct BombExploded Buffer::receive_bomb_exploded() {
	struct BombExploded event_content;
	receive(event_content.bomb_id);
	receive_list_player_ids(event_content.robots_destroyed);
	receive_list_positions(event_content.blocks_destroyed);

	std:: cerr << "received bomb_id: "<< event_content.bomb_id << std::endl;
	std:: cerr << "received robots_destroyed size : "<< event_content.robots_destroyed.size() << std::endl;
	std:: cerr << "received robots_destroyed: ";
	for(int i : event_content.robots_destroyed) {
		std:: cerr << i << std::endl;
	}
	std:: cerr << "received blocks_destroyed size : "<< event_content.blocks_destroyed.size() << std::endl;
	for(auto i : event_content.blocks_destroyed) {
		std:: cerr << i.x << " " << i.y << std::endl;
	}

	return event_content;
}

struct PlayerMoved Buffer::receive_player_moved() {
	struct PlayerMoved event_content;
	receive(event_content.player_id);
	receive(event_content.position);
	return event_content;
}

struct BlockPlaced Buffer::receive_block_placed() {
	struct BlockPlaced event_content;
	receive(event_content.position);
	return event_content;
}

Event Buffer::receive_event() {
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
}

void Buffer::receive_list_events(std::vector<Event> &vector) {
	uint32_t vector_size;
	receive(vector_size);
	for (uint32_t i = 0; i < vector_size; ++i) {
		vector.push_back(receive_event());
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

struct Hello Buffer::receive_hello() {
	struct Hello hello;
	receive(hello.server_name);
	receive(hello.players_count);
	receive(hello.size_x);
	receive(hello.size_y);
	receive(hello.game_length);
	receive(hello.explosion_radius);
	receive(hello.bomb_timer);
	return hello;
}

struct AcceptedPlayer Buffer::receive_accepted_player() {
	struct AcceptedPlayer accepted_player;
	receive(accepted_player.player_id);
	receive(accepted_player.player);
	return accepted_player;
}

struct GameStarted Buffer::receive_game_started() {
	struct GameStarted game_started;
	receive_map_players(game_started.players);
	return game_started;
}

struct Turn Buffer::receive_turn() {
	struct Turn turn;
	receive(turn.turn_number);
	receive_list_events(turn.events);
	return turn;
}

struct GameEnded Buffer::receive_game_ended() {
	struct GameEnded game_ended;
	receive_map_scores(game_ended.scores);
	return game_ended;
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
	print(get_send_size());
	return get_send_size();
}

std::optional<ServerMessageToClient>
Buffer::receive_msg_from_server() {
	reset_read_index();
	auto serverMessage = std::optional<ServerMessageToClient>();
	uint8_t message;
	receive(message);
	if (invalid_server_message_type(message)) {
		return {};
	}
	std::variant<struct Hello, struct AcceptedPlayer,
			struct GameStarted, struct Turn, struct GameEnded> data;
	switch ((ServerMessageToClientType) message) {
		case Hello:
			data = receive_hello();
			serverMessage.emplace(ServerMessageToClientType::Hello, data);
			break;
		case AcceptedPlayer:
			data = receive_accepted_player();
			serverMessage.emplace(ServerMessageToClientType::AcceptedPlayer,
			                      data);
			break;
		case GameStarted:
			data = receive_game_started();
			serverMessage.emplace(ServerMessageToClientType::GameStarted, data);
			break;
		case Turn:
			data = receive_turn();
			serverMessage.emplace(ServerMessageToClientType::Turn, data);
			break;
		case GameEnded:
			data = receive_game_ended();
			serverMessage.emplace(ServerMessageToClientType::GameEnded, data);
			break;
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

std::optional<DisplayMessageToClient>
Buffer::receive_msg_from_gui(size_t expected_size) {
	auto message = std::optional<DisplayMessageToClient>();
	reset_read_index();

	uint8_t message_type;
	uint8_t direction;

	receive(message_type);

	if (message_type == DisplayMessageToClientType::MoveDisplay) {
		receive(direction);
		if (invalid_direction(direction)) {
			return {};
		}

		message.emplace((DisplayMessageToClientType) message_type,
		                static_cast<Direction>(direction));
		return message;
	}

	if (invalid_gui_message_type(message_type) ||
	    get_read_size() != expected_size) {
		return {};
	}

	message.emplace((DisplayMessageToClientType) message_type);
	return message;
}


