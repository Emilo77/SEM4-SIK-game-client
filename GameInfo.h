//
// Created by emilo77 on 10.05.22.
//

#ifndef ZADANIE02_CLIENT_GAMEINFO_H
#define ZADANIE02_CLIENT_GAMEINFO_H

#include <string>
#include "Utils.h"



class GameInfo {
private:
	enum GameState gameState{Lobby};
	std::string server_name{};
	uint8_t players_count{0};
	uint16_t size_x{0};
	uint16_t size_y{0};
	uint16_t game_length{0};
	uint16_t explosion_radius{0};
	uint16_t bomb_timer{0};
	std::map<player_id_t, Player> players;

	//Gameplay state
	uint16_t turn{0};
	std::map<player_id_t, Position> player_positions;
	std::list<Position> blocks;
	std::list<Bomb> bombs;
	std::list<Position> explosions;
	std::map<player_id_t, score_t> scores;
};


#endif//ZADANIE02_CLIENT_GAMEINFO_H
