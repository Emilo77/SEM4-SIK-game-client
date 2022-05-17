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
	players.insert({message.player_id, message.player});
}

void GameInfo::apply_GameStarted(struct GameStarted &message) {
	game_state = GameState::Gameplay;
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
	bombs.emplace_back(data.bomb_id, data.position);
}

void GameInfo::apply_BombExploded(struct BombExploded &data) {
	//calculate exploded blocks

	for (auto robot_id: data.robots_destroyed) {
		scores.at(robot_id)++;
	}
	for (auto block: data.blocks_destroyed) {
		blocks.erase(block);
	}
}

void GameInfo::apply_PlayerMoved(struct PlayerMoved &data) {
	player_positions.at(data.player_id) = data.position;
}


void GameInfo::apply_BlockPlaced(struct BlockPlaced &data) {
	blocks.insert(data.position);
}


void GameInfo::apply_event(Event &event) {
	switch (event.type) {
		case BombPlaced:
			break;
		case BombExploded:
			break;
		case PlayerMoved:
			break;
		case BlockPlaced:
			break;
	}
}


void GameInfo::restart_info() {
	game_state = GameState::Lobby;
	server_name.clear();
	players.clear();
	player_positions.clear();
	blocks.clear();
	bombs.clear();
	explosions.clear();
	scores.clear();
}

struct Lobby GameInfo::create_lobby_msg() {
	return {};
}

struct GamePlay GameInfo::create_gameplay_msg() {
	return {};
}
