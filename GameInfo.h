//
// Created by emilo77 on 10.05.22.
//

#ifndef ZADANIE02_CLIENT_GAMEINFO_H
#define ZADANIE02_CLIENT_GAMEINFO_H

#include <string>
#include "Utils.h"



class GameInfo {
private:
	enum GameState game_state{Lobby};
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

	void restart_info();

	void apply_event(Event &event);

	void apply_Hello(struct Hello &message);

	void apply_AcceptedPlayer(struct AcceptedPlayer &message);

	void apply_GameStarted(struct GameStarted &message);

	void apply_Turn(struct Turn &message);

	void apply_GameEnded(struct GameEnded &message);

	void place_bomb(Position position);


public:

	void apply_changes_from_server(ServerMessageToClient &msg);



};


#endif//ZADANIE02_CLIENT_GAMEINFO_H
