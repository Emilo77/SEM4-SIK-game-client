#ifndef ZADANIE02_BUFFER_H
#define ZADANIE02_BUFFER_H

#define BUFFER_SIZE 65507

#define MAX_SERVER_INPUT_TYPE 3
#define MAX_DISPLAY_INPUT_TYPE 2
#define MAX_DIRECTION_TYPE 3

#include "ClientParameters.h"
#include "Utils.h"
#include <cstring>
#include <optional>


using std::string;

class Buffer {
private:
	//CONVERTING NUMBERS
	template<typename T>
	T convert_to_send(T number);
	template<typename T>
	T convert_to_receive(T number);

	//RAW INSERTS
	void insert_raw(const std::string &str);

	//RAW RECEIVES
	void receive_raw(std::string &str, size_t str_size);
public:
	//INSERTS WITH TASK CONVENTION
	template<typename T>
	void insert(T number);
	void insert(Direction direction);
	void insert(const std::string &str);
	void insert(Position &position);
	void insert(Player &player);
	void insert(Bomb &bomb);
	template<typename T>
	void insert_list(std::list<T> &list);
	template<typename T, typename U>
	void insert_map(std::map<T, U> &map);

	// RECEIVES WITH TASK CONVENTION
	template<typename T>
	void receive(T &number);
	void receive(std::string &str);
	void receive(Position &position);
	void receive(Player &player);
	void receive(Bomb &bomb);

	void receive_event_content(struct BombPlaced &data);
	void receive_event_content(struct BombExploded &data);
	void receive_event_content(struct PlayerMoved &data);
	void receive_event_content(struct BlockPlaced &data);
	void receive_event(EventType type, Event &event);

	void receive_event_list(std::vector<Event> &vector);
	template<typename T>
	void receive_list(std::vector<T> &vector);
	template<typename T, typename U>
	void receive_map(std::map<T, U> &map);


	// SENDING TO SERVER
	void send_join(std::string &string);
	void send_place_bomb();
	void send_place_block();
	void send_move(Direction direction);

	// RECEIVING FROM SERVER
	size_t receive_hello(struct Hello &message);
	size_t receive_accepted_player(struct AcceptedPlayer &message);
	size_t receive_game_started(struct GameStarted &message);
	size_t receive_turn(struct Turn &message);
	size_t receive_game_ended(struct GameEnded &message);

	// SENDING TO DISPLAY
	void send_lobby(struct Lobby &message);
	void send_game(struct GamePlay &message);

public:
	void reset_read_index() { read_index = 0; }
	void reset_send_index() { send_index = 0; }

	size_t send_to_server(ClientMessageToServer &message);

	size_t send_to_display(ClientMessageToDisplay &drawMessage);

	std::optional<ServerMessageToClient> receive_from_server(size_t length);

	std::optional<DisplayMessageToClient> receive_from_display(size_t length);

	size_t get_send_size() const { return send_index; }

	size_t get_read_size() const { return read_index; }

	char get_message_id() { return buffer[0]; }

	char *get() { return buffer; }

private:
	char buffer[BUFFER_SIZE]{};
	size_t send_index{0};
	size_t read_index{0};
};


#endif//ZADANIE02_BUFFER_H
