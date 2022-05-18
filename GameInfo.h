#ifndef ZADANIE02_CLIENT_GAMEINFO_H
#define ZADANIE02_CLIENT_GAMEINFO_H

#include <string>
#include <set>
#include <map>
#include "Utils.h"
#include "Messages.h"
#include <iostream>

class Field {
public:
	bool is_solid{false};

	void make_block() {
		is_solid = true;
	}

	void make_air() {
		is_solid = false;
	}

};

class Board {

	std::vector<std::vector<Field>> fields;
public:
	void reset(uint16_t size_x, uint16_t size_y) {
		fields.resize(size_x);
		for (auto &row: fields) {
			row.resize(size_y);
		}

		for (auto &column: fields) {
			for (auto &field: column) {
				field.make_air();
			}
		}
	}

	void place_block(Position position) {
		fields[position.x][position.y].make_block();
	}

	void explode_block(Position position) {
		fields[position.x][position.y].make_air();
	}

	std::list<Position> return_blocks() {
		std::list<Position> blocks;
		for (size_t column = 0; column < fields.size(); column++) {
			for (size_t row = 0; row < fields[column].size(); row++) {
				if (fields[column][row].is_solid) {
					blocks.emplace_back(column, row);
				}
			}
		}
		return blocks;
	}

};

class GameInfo {
private:
	enum GameState game_state{LobbyState};
	std::string server_name{};
	uint8_t players_count{0};
	uint16_t size_x{0};
	uint16_t size_y{0};
	uint16_t game_length{0};
	uint16_t explosion_radius{0};
	uint16_t bomb_timer{0};
	std::map<player_id_t, Player> players;

	//GameplayState state
	Board board;
	uint16_t turn{0};
	std::map<player_id_t, Position> player_positions;
	std::map<bomb_id_t, Bomb> bombs;
	std::map<player_id_t, score_t> scores;
	std::list<Position> explosions;

	void restart_info();

	void decrease_bomb_timers();

	std::list<Position> calculate_explosion(struct BombExploded &data);

	void apply_event(Event &event);

	void apply_Hello(struct Hello &message);

	void apply_AcceptedPlayer(struct AcceptedPlayer &message);

	void apply_GameStarted(struct GameStarted &message);

	void apply_Turn(struct Turn &message);

	void apply_GameEnded(struct GameEnded &message);

	void apply_BombPlaced(struct BombPlaced &data);

	void apply_BombExploded(struct BombExploded &data);

	void apply_PlayerMoved(struct PlayerMoved &data);

	void apply_BlockPlaced(struct BlockPlaced &data);


public:

	void apply_changes_from_server(ServerMessageToClient &msg);

	bool is_gameplay() { return game_state == GameplayState; }

	Lobby create_lobby_msg();

	GamePlay create_gameplay_msg();
};


#endif//ZADANIE02_CLIENT_GAMEINFO_H
