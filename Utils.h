#ifndef ZADANIE02_EVENT_H
#define ZADANIE02_EVENT_H

#include <csignal>
#include <cstdint>
#include <variant>
#include <vector>
#include <list>

using player_id_t = uint8_t;
using bomb_id_t = uint32_t;
using score_t = uint32_t;

enum Direction {
	Up = 0,
	Right = 1,
	Down = 2,
	Left = 3,
};

enum GameState {
	Lobby = 0,
	Gameplay = 1,
};


struct Position {
	uint16_t x;
	uint16_t y;

	Position() : x(0), y(0) {}

	Position(uint16_t x, uint16_t y) : x(x), y(y) {}

	bool operator==(const Position &rhs) const {
		return x == rhs.x &&
		       y == rhs.y;
	}

	bool operator!=(const Position &rhs) const {
		return !(x == rhs.x && y == rhs.y);
	}

	bool operator<(const Position &rhs) const {
		if (x < rhs.x)
			return true;
		if (rhs.x < x)
			return false;
		return y < rhs.y;
	}

	bool operator>(const Position &rhs) const {
		return rhs < *this;
	}

	bool operator<=(const Position &rhs) const {
		return !(rhs < *this);
	}

	bool operator>=(const Position &rhs) const {
		return !(*this < rhs);
	}
};

class Bomb {
public:
	bomb_id_t bomb_id;
	Position position;
	uint16_t timer;

	Bomb(bomb_id_t bomb_id, Position position, uint16_t timer)
			: bomb_id(bomb_id),
			  position(position), timer(timer) {}

	void decrease_timer() { timer--; }

	bool operator<(const Bomb &rhs) const { return bomb_id < rhs.bomb_id; }
};

struct Player {
	std::string name;
	std::string address;
};

static std::pair<int, int> direction_to_pair(Direction &direction) {
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
	return {69, 69};
}

enum EventType {
	BombPlaced = 0,
	BombExploded = 1,
	PlayerMoved = 2,
	BlockPlaced = 3,
};

struct BombPlaced {
	uint8_t bomb_id;
	Position position;
};
struct BombExploded {
	uint8_t bomb_id;
	std::vector<uint8_t> robots_destroyed;
	std::vector<Position> blocks_destroyed;
};
struct PlayerMoved {
	uint8_t player_id;
	Position position;
};
struct BlockPlaced {
	Position position;
};

class Event {
public:
	EventType type;
	std::variant<struct BombPlaced, struct BombExploded,
			struct PlayerMoved, struct BlockPlaced> data;

	explicit Event(EventType type,
	               std::variant<struct BombPlaced, struct BombExploded,
			               struct PlayerMoved, struct BlockPlaced> &data)
			: type(type), data(data) {
	}
};

static bool invalid_direction(uint8_t direction) {
	return direction > 3;
}

static bool invalid_server_message_type(uint8_t type) {
	return type > 4;
}

static bool invalid_display_message_type(uint8_t type) {
	return type > 2;
}

static bool invalid_game_state(uint8_t state) {
	return state > 1;
}


#endif//ZADANIE02_EVENT_H
