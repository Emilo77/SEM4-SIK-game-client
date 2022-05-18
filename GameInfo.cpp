#include "GameInfo.h"

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
	restart_info();
	server_name = message.server_name;
	players_count = message.players_count;
	size_x = message.size_x;
	size_y = message.size_y;
	game_length = message.game_length;
	explosion_radius = message.explosion_radius;
	bomb_timer = message.bomb_timer;
}

void GameInfo::apply_AcceptedPlayer(struct AcceptedPlayer &message) {
	players.insert_or_assign(message.player_id, message.player);
}

void GameInfo::apply_GameStarted(struct GameStarted &message) {
	board.reset(size_x, size_y);
	game_state = GameState::GameplayState;
	players.clear();
	players = message.players;
}

void GameInfo::apply_Turn(struct Turn &message) {
	turn = message.turn;
	for (auto &event: message.events) {
		apply_event(event);
	}
}

void GameInfo::apply_GameEnded(struct GameEnded &message) {
	scores = message.scores;
}

void GameInfo::apply_BombPlaced(struct BombPlaced &data) {
	bombs.insert({data.bomb_id, Bomb(data.bomb_id, data.position, bomb_timer)});
}

void GameInfo::apply_BombExploded(struct BombExploded &data) {
	std::list<Position> new_explosions = calculate_explosion(data);
	explosions = new_explosions;

	for (auto robot_id: data.robots_destroyed) {
		if (scores.find(robot_id) == scores.end()) {
			std::cerr << "Player not found!" << std::endl;
		} else {
			scores.at(robot_id)++;
		}
	}

	for (auto &position: data.blocks_destroyed) {
		board.explode_block(position);
	}

	bombs.erase(data.bomb_id);
}

void GameInfo::apply_PlayerMoved(struct PlayerMoved &data) {
	if (players.find(data.player_id) == players.end()) {
		std::cerr << "Player not found!" << std::endl;
	} else {
		player_positions.at(data.player_id) = data.position;
	}
}


void GameInfo::apply_BlockPlaced(struct BlockPlaced &data) {
	board.place_block(data.position);
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


void GameInfo::restart_info() {
	board.reset(size_x, size_y);
	game_state = GameState::LobbyState;
	server_name.clear();
	players.clear();
	player_positions.clear();
	bombs.clear();
	scores.clear();
}

void GameInfo::decrease_bomb_timers() {
	for(auto &bombs_element : bombs) {
		bombs_element.second.decrease_timer();
	}
}


std::list<Position> GameInfo::calculate_explosion(struct BombExploded &data) {
	std::list<Position> exploded;
	Position bomb_pos = bombs.at(data.bomb_id).position;
	exploded.push_back(bomb_pos);

//	for(int i = 1; i <= explosion_radius; i++) {
//
//	}
	//todo

	return exploded;
}

Lobby GameInfo::create_lobby_msg() {
	return {server_name, players_count, size_x, size_y, game_length,
	        explosion_radius, bomb_timer, players};
}

struct GamePlay GameInfo::create_gameplay_msg() {
	std::list<Position> blocks = board.return_blocks();
	std::list<Bomb> bombs_vector;
	for (auto &bomb: bombs) {
		bombs_vector.push_back(bomb.second);
	}

	return {server_name, size_x, size_y, game_length, turn, players,
	        player_positions, blocks, bombs_vector, explosions, scores};
}
