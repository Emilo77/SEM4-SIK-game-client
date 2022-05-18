#ifndef ZADANIE02_BUFFER_H
#define ZADANIE02_BUFFER_H

#define BUFFER_SIZE 65507

#include "ClientParameters.h"
#include "Utils.h"
#include "Messages.h"
#include <cstring>
#include <optional>


using std::string;

class Buffer {
private:
	//CONVERTING NUMBERS TO SEND
	static uint8_t convert_to_send(uint8_t number);

	static uint16_t convert_to_send(uint16_t number);

	static uint32_t convert_to_send(uint32_t number);

	static uint64_t convert_to_send(uint64_t number);

	//CONVERTING NUMBERS TO SEND
	static uint8_t convert_to_receive(uint8_t number);

	static uint16_t convert_to_receive(uint16_t number);

	static uint32_t convert_to_receive(uint32_t number);

	static uint64_t convert_to_receive(uint64_t number);

	//RAW INSERTS
	void insert_raw(const std::string &str);

	//RAW RECEIVES
	void receive_raw(std::string &str, size_t str_size);

	//INSERTS NUMERIC
	void insert(uint8_t number);

	void insert(uint16_t number);

	void insert(uint32_t number);

	void insert(uint64_t number);

	//INSERTS WITH TASK CONVENTION
	void insert(const std::string &str);

	void insert(Position &position);

	void insert(Player &player);

	void insert(Bomb &bomb);

	//INSERTS LISTS
	void insert_list_positions(std::list<Position> &positions);

	void insert_list_bombs(std::list<Bomb> &bombs);

	//INSERTS MAPS
	void insert_map_players(std::map<player_id_t, Player> &players);

	void insert_map_scores(std::map<player_id_t, score_t> &scores);

	void insert_map_positions(std::map<player_id_t, Position> &positions);

	// RECEIVES NUMERIC
	void receive(uint8_t &number);

	void receive(uint16_t &number);

	void receive(uint32_t &number);

	void receive(uint64_t &number);

	// RECEIVES WITH TASK CONVENTION
	void receive(std::string &str);

	void receive(Position &position);

	void receive(Player &player);

	// UTILITIES TO RECEIVE EVENT
	void receive_event_content(struct BombPlaced &data);

	void receive_event_content(struct BombExploded &data);

	void receive_event_content(struct PlayerMoved &data);

	void receive_event_content(struct BlockPlaced &data);

	void receive_event(EventType type, Event &event);

	// RECEIVES LISTS
	void receive_list_events(std::vector<Event> &vector);

	void receive_list_player_ids(std::vector<player_id_t> &ids);

	void receive_list_positions(std::vector<Position> &positions);

	// RECEIVES MAPS
	void receive_map_players(std::map<player_id_t, Player> &players);

	void receive_map_scores(std::map<player_id_t, score_t> &scores);

	// SENDING TO SERVER
	void insert_join(std::string &string);

	void insert_place_bomb();

	void insert_place_block();

	void insert_move(Direction direction);

	// RECEIVING FROM SERVER
	size_t receive_hello(struct Hello &message);

	size_t receive_accepted_player(struct AcceptedPlayer &message);

	size_t receive_game_started(struct GameStarted &message);

	size_t receive_turn(struct Turn &message);

	size_t receive_game_ended(struct GameEnded &message);

	// SENDING TO DISPLAY
	void send_lobby(Lobby &message);

	void send_game(GamePlay &message);

public:
	void reset_read_index() { read_index = 0; }

	void reset_send_index() { send_index = 0; }

	size_t insert_msg_to_server(ClientMessageToServer &message);

	std::optional<ServerMessageToClient>
	receive_msg_from_server(size_t length);

	size_t insert_msg_to_display(ClientMessageToDisplay &drawMessage);

	std::optional<DisplayMessageToClient>
	receive_msg_from_display(size_t length);

	[[nodiscard]] size_t get_send_size() const { return send_index; }

	[[nodiscard]] size_t get_read_size() const { return read_index; }

	char *get() { return buffer; }

private:
	char buffer[BUFFER_SIZE]{};
	size_t send_index{0};
	size_t read_index{0};
};


#endif//ZADANIE02_BUFFER_H
