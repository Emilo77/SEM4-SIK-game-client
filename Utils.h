#ifndef ZADANIE02_EVENT_H
#define ZADANIE02_EVENT_H

#include <cstdint>
#include <string>
#include <map>
#include <list>
#include <bits/sigaction.h>
#include <csignal>

using player_id_t = uint8_t;
using bomb_id_t = uint32_t;
using score_t = uint32_t;

enum Direction {
	Up = 0,
	Right = 1,
	Down = 2,
	Left = 3,
	InvalidDirection = 4,
};

enum ClientMessage {
	Join = 0,
	PlaceBomb = 1,
	PlaceBlock = 2,
	Move = 3,
	Invalid = 4,
	// przy castowaniu na enum, dowolna inna wartość niż <1,4>
	// stanie siże zerem, czyli Invalid
};

struct Player {
	std::string name;
	std::string address;
};

struct Position {
	uint16_t x;
	uint16_t y;

	Position(uint16_t x, uint16_t y) : x(x), y(y) {}

	bool operator==(const Position &rhs) const {
		return x == rhs.x &&
		       y == rhs.y;
	}

	bool operator!=(const Position &rhs) const {
		return !(x == rhs.x &&
		         y == rhs.y);
	}
};

class Bomb {
	uint32_t bomb_id;
	uint16_t timer;
	Position position;

public:
	bool operator<(const Bomb &rhs) const {
		return bomb_id < rhs.bomb_id;
	}

	void tic() {
		timer--;
	}

	bool will_explode() {
		return timer == 0;
	}

	bomb_id_t get_id() {
		return bomb_id;
	}

	Position get_position() {
		return position;
	}
};

struct DrawMessage {
	std::string server_name;
	uint16_t size_x;
	uint16_t size_y;
	uint16_t game_length;
	std::map<player_id_t , Player> players;

	// [0] Lobby
	uint8_t players_count;
	uint16_t explosion_radius;
	uint16_t bomb_timer;

	// [1] Game
	uint16_t turn;
	std::map<player_id_t, Position> player_positions;
	std::list<Position> blocks;
	std::list<Bomb> bombs;
	std::list<Position> explosions;
	std::map<player_id_t, score_t> scores;
};



std::pair<int,int> direction_to_pair(Direction &direction) {
	switch(direction) {
		case Up:
			return {0, 1};
		case Right:
			return {1, 0};
		case Down:
			return {0, -1};
		case Left:
			return {-1, 0};
		case InvalidDirection:
			return {69, 69}; //todo potem zmienić
	}
}

class Event {
};

class BombPlaced : public Event {
public:
	uint32_t bomb_id;
	Position position;
};

class BombExploded : public Event {
public:
	uint32_t bomb_id;
	std::list<uint8_t> robots_destroyed;
	std::list<Position> blocks_destroyed;
};

class PlayerMoved : public Event {
public:
	uint8_t player_id;
	Position position;
};

class BlockPlaced : public Event {
public:
	Position position;
};

#endif //ZADANIE02_EVENT_H
