#include "GameInfo.h"

static std::pair<int, int> direction_to_pair(Direction direction) {
	switch (direction) {
		case Up:
			return {0, 1};
		case Right:
			return {1, 0};
		case Down:
			return {0, -1};
		case Left:
			return {-1, 0};
	}
	return {0, 0};
}

void GameInfo::apply_changes_from_server(ServerMessageToClient &msg) {
	switch (msg.type) {
		case Hello:
			apply_Hello(std::get<struct Hello>(msg.data));
			break;
		case AcceptedPlayer:
			apply_AcceptedPlayer(std::get<struct AcceptedPlayer>(msg.data));
			break;
		case GameStarted:
			apply_GameStarted(std::get<struct GameStarted>(msg.data));
			break;
		case Turn:
			apply_Turn(std::get<struct Turn>(msg.data));
			break;
		case GameEnded:
			apply_GameEnded(std::get<struct GameEnded>(msg.data));
			break;
	}
}

void GameInfo::apply_Hello(struct Hello &message) {
	clear_containers();

	server_name = message.server_name;
	players_count = message.players_count;
	size_x = message.size_x;
	size_y = message.size_y;
	game_length = message.game_length;
	explosion_radius = message.explosion_radius;
	bomb_timer = message.bomb_timer;

	board.reset(message.size_x, message.size_y);
}

void GameInfo::apply_AcceptedPlayer(struct AcceptedPlayer &message) {
	players.insert_or_assign(message.player_id, message.player);
}

void GameInfo::apply_GameStarted(struct GameStarted &message) {
	change_game_state(GameState::GameplayState);
	players = message.players;
	initialize_containers();
}

void GameInfo::apply_Turn(struct Turn &message) {
	explosions.clear();
	decrease_bomb_timers(); //może nie w tym miejscu
	turn = message.turn_number;
	for (auto &event: message.events) {
		apply_event(event);
	}
	change_scores_and_revive_players();
	explosions = board.return_explosions();
}

void GameInfo::apply_GameEnded(struct GameEnded &message) {
	scores = message.scores;
	change_game_state(GameState::LobbyState);
	clear_containers(); // może niepotrzebne
}

void GameInfo::apply_BombPlaced(struct BombPlaced &data) {
	bombs.insert({data.bomb_id, Bomb(data.bomb_id, data.position, bomb_timer)});
	std::cerr << "Bomb placed at " << data.position.x << " " << data.position.y << std::endl;
}

void GameInfo::apply_BombExploded(struct BombExploded &data) {
	mark_explosions(data);

	for (auto robot_id: data.robots_destroyed) {
		if (players.find(robot_id) != players.end()) {
			players.at(robot_id).explode();
		}
	}

	for (auto &position: data.blocks_destroyed) {
		board.at(position).make_air();
	}

	bombs.erase(data.bomb_id);
}

void GameInfo::apply_PlayerMoved(struct PlayerMoved &data) {
	if (players.find(data.player_id) != players.end()) {
		player_positions.at(data.player_id) = data.position;
	}
}

void GameInfo::apply_BlockPlaced(struct BlockPlaced &data) {
	board.at(data.position).make_block();
}

void GameInfo::apply_event(Event &event) {
	switch (event.type) {
		case BombPlaced:
			apply_BombPlaced(std::get<struct BombPlaced>(event.data));
			break;
		case BombExploded:
			apply_BombExploded(std::get<struct BombExploded>(event.data));
			break;
		case PlayerMoved:
			apply_PlayerMoved(std::get<struct PlayerMoved>(event.data));
			break;
		case BlockPlaced:
			apply_BlockPlaced(std::get<struct BlockPlaced>(event.data));
			break;
	}
}

bool GameInfo::is_correct_position(Position position) const {
	return position.x < size_x && position.y < size_y;
}

void GameInfo::clear_containers() {
	players.clear();
	player_positions.clear();
	bombs.clear();
	scores.clear();
	explosions.clear();

}

void GameInfo::initialize_containers() {
	for (auto &new_player_pair: players) {
		scores.insert({new_player_pair.first, 0});
		player_positions.insert({new_player_pair.first, Position()});
	}
}

void GameInfo::decrease_bomb_timers() {
	for (auto &bombs_element: bombs) {
		bombs_element.second.decrease_timer();
	}
}

void GameInfo::change_scores_and_revive_players() {
	for (auto &player_pair: players) {
		if (player_pair.second.is_dead()) {
			scores.at(player_pair.first)++;
			player_pair.second.revive();
		}
	}
}

void GameInfo::mark_explosions_in_direction(Position &bomb_pos, Direction direction) {

	auto pair = direction_to_pair(direction);

	for (int i = 1; i <= explosion_radius; i++) {
		Position new_pos(static_cast<uint16_t>(bomb_pos.x + i * pair.first),
		                 static_cast<uint16_t>(bomb_pos.x + i * pair.second));

		if (is_correct_position(new_pos)) {
			board.at(new_pos).mark_exploded();
		}
		if (board.at(new_pos).is_solid()) {
			break;
		}
	}
}


void GameInfo::mark_explosions(struct BombExploded &data) {

	Position bomb_pos = bombs.at(data.bomb_id).position;

	board.at(bomb_pos).mark_exploded();

	if (board.at(bomb_pos).is_solid()) {
		return;
	}

	mark_explosions_in_direction(bomb_pos, Up);
	mark_explosions_in_direction(bomb_pos, Right);
	mark_explosions_in_direction(bomb_pos, Down);
	mark_explosions_in_direction(bomb_pos, Left);

}

bool GameInfo::is_gameplay() {
	bool result = false;
	game_protection.lock();
	if (game_state == GameplayState) {
		result = true;
	}
	game_protection.unlock();
	return result;
}

void GameInfo::change_game_state(GameState state) {
	game_protection.lock();
	game_state = state;
	game_protection.unlock();
}

Lobby GameInfo::create_lobby_msg() {
	return {server_name, players_count, size_x, size_y, game_length,
	        explosion_radius, bomb_timer, players};
}

struct GamePlay GameInfo::create_gameplay_msg() {
	std::list<Position> blocks = board.return_blocks();
	std::list<Bomb> bombs_list;
	for (auto &bomb: bombs) {
		bombs_list.push_back(bomb.second);
	}

	return {server_name, size_x, size_y, game_length, turn, players,
	        player_positions, blocks, bombs_list, explosions, scores};
}
