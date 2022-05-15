#ifndef ZADANIE02_EVENT_H
#define ZADANIE02_EVENT_H

#include <csignal>
#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <variant>

using player_id_t = uint8_t;
using bomb_id_t = uint32_t;
using score_t = uint32_t;

class Event {
};

enum Direction {
	Up = 0,
	Right = 1,
	Down = 2,
	Left = 3,
};
static bool invalid_direction(uint8_t direction) {
	return direction > 3;
}

enum ClientMessageToServerType {
	JoinServer = 0,
	PlaceBombServer = 1,
	PlaceBlockServer = 2,
	MoveServer = 3,
};

enum ServerMessageToClientType {
	Hello = 0,
	AcceptedPlayer = 1,
	GameStarted = 2,
	Turn = 3,
	GameEnded = 4,
};
static bool invalid_server_message_type(uint8_t type) {
	return type > 4;
}

enum DisplayMessageToClientType {
	PlaceBombDisplay = 0,
	PlaceBlockDisplay = 1,
	MoveDisplay = 2,
};
static bool invalid_display_message_type(uint8_t type) {
	return type > 2;
}

enum GameState {
	Lobby = 0,
	Gameplay = 1,
};
static bool invalid_game_state(uint8_t state) {
	return state > 1;
}

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
		return !(x == rhs.x && y == rhs.y);
	}
};

class Bomb {
public:
	bomb_id_t bomb_id;
	uint16_t timer;
	Position position;

	bool operator<(const Bomb &rhs) const { return bomb_id < rhs.bomb_id; }
};

struct Hello {
	std::string server_name;
	uint8_t players_count;
	uint16_t size_x;
	uint16_t size_y;
	uint16_t game_length;
	uint16_t explosion_radius;
	uint16_t bomb_timer;
};

struct AcceptedPlayer {
	player_id_t player_id{};
	Player player{};
};

struct GameStarted {
	std::map<player_id_t, Player> players;
};

struct Turn {
	uint16_t turn;
	std::list<Event> events;
};

struct GameEnded {
	std::map<player_id_t, score_t> scores;
};


struct Lobby {
	std::string server_name;
	uint8_t players_count;
	uint16_t size_x;
	uint16_t size_y;
	uint16_t game_length;
	uint16_t explosion_radius;
	uint16_t bomb_timer;
	std::map<player_id_t, Player> players;
};

struct GamePlay {
	std::string server_name;
	uint16_t size_x;
	uint16_t size_y;
	uint16_t game_length;
	uint16_t turn;
	std::map<player_id_t, Player> players;
	std::map<player_id_t, Position> player_positions;
	std::list<Position> blocks;
	std::list<Bomb> bombs;
	std::list<Position> explosions;
	std::map<player_id_t, score_t> scores;
};


struct ServerMessageToClient {
	ServerMessageToClientType type;
	std::variant<struct Hello, struct AcceptedPlayer,
	             struct GameStarted, struct Turn, struct GameEnded>
	        data;
};

struct DisplayMessageToClient {
	DisplayMessageToClientType type;
	Direction direction;
};

struct ClientMessageToServer {
	ClientMessageToServerType type;
	std::variant<std::string, Direction> data;
};


struct ClientMessageToDisplay {
	GameState state;
	std::variant<struct Lobby, struct GamePlay> data;
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

class BombPlaced : public Event {
public:
	bomb_id_t bomb_id;
	Position position;
};

class BombExploded : public Event {
public:
	bomb_id_t bomb_id;
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

#endif//ZADANIE02_EVENT_H
