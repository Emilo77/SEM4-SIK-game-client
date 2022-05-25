#ifndef ZADANIE02_CLIENT_MESSAGES_H
#define ZADANIE02_CLIENT_MESSAGES_H

#include <utility>
#include <variant>
#include <string>
#include <cstdint>
#include <map>
#include "Utils.h"


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

enum DisplayMessageToClientType {
	PlaceBombDisplay = 0,
	PlaceBlockDisplay = 1,
	MoveDisplay = 2,
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
	uint16_t turn_number;
	std::vector<Event> events;
};

struct GameEnded {
	std::map<player_id_t, score_t> scores;
};


typedef struct Lobby {
	std::string server_name;
	uint8_t players_count;
	uint16_t size_x;
	uint16_t size_y;
	uint16_t game_length;
	uint16_t explosion_radius;
	uint16_t bomb_timer;
	std::map<player_id_t, Player> players;

	Lobby(std::string &serverName, uint8_t playersCount, uint16_t sizeX,
	      uint16_t sizeY, uint16_t gameLength, uint16_t explosionRadius,
	      uint16_t bombTimer, std::map<player_id_t, Player> &players)
			: server_name(serverName),
			  players_count(playersCount),
			  size_x(sizeX),
			  size_y(sizeY),
			  game_length(gameLength),
			  explosion_radius(explosionRadius),
			  bomb_timer(bombTimer),
			  players(players) {}
} Lobby;

typedef struct GamePlay {
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

	GamePlay(std::string &serverName, uint16_t sizeX, uint16_t sizeY,
	         uint16_t gameLength, uint16_t turn,
	         std::map<player_id_t, Player> &players,
	         std::map<player_id_t, Position> &playerPositions,
	         std::list<Position> &blocks,
	         std::list<Bomb> &bombs,
	         std::list<Position> &explosions,
	         std::map<player_id_t, score_t> &scores)
			: server_name(serverName),
			  size_x(sizeX),
			  size_y(sizeY),
			  game_length(gameLength),
			  turn(turn),
			  players(players),
			  player_positions(playerPositions),
			  blocks(blocks),
			  bombs(bombs),
			  explosions(explosions),
			  scores(scores) {}
} GamePlay;


typedef struct ServerMessageToClient {
	ServerMessageToClientType type;
	std::variant<struct Hello, struct AcceptedPlayer,
			struct GameStarted, struct Turn, struct GameEnded>
			data;

	ServerMessageToClient(ServerMessageToClientType type,
						  std::variant<struct Hello, struct AcceptedPlayer,
									  struct GameStarted, struct Turn, struct GameEnded>
									  data) : type(type), data(std::move(data)) {}

//	ServerMessageToClient(ServerMessageToClientType type, struct Hello &data)
//			: type(type), data(data) {}
//
//	ServerMessageToClient(ServerMessageToClientType type,
//	                      struct AcceptedPlayer &data)
//			: type(type), data(data) {}
//
//	ServerMessageToClient(ServerMessageToClientType type,
//	                      struct GameStarted &data)
//			: type(type), data(data) {}
//
//	ServerMessageToClient(ServerMessageToClientType type, struct Turn &data)
//			: type(type), data(data) {}
//
//	ServerMessageToClient(ServerMessageToClientType type,
//	                      struct GameEnded &data)
//			: type(type), data(data) {}

} ServerMessageToClient;

typedef struct DisplayMessageToClient {
	DisplayMessageToClientType type;
	Direction direction;
} DisplayMessageToClient;

typedef struct ClientMessageToServer {
	ClientMessageToServerType type;
	std::variant<std::string, Direction> data;
} ClientMessageToServer;


typedef struct ClientMessageToDisplay {
	GameState state;
	std::variant<struct Lobby, struct GamePlay> data;

	ClientMessageToDisplay(GameState state,
	                       std::variant<struct Lobby, struct GamePlay> data)
			: state(state), data(std::move(data)) {}
} ClientMessageToDisplay;


#endif //ZADANIE02_CLIENT_MESSAGES_H
