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
	void reset_read_index() { read_index = 0; }
	void reset_send_index() { send_index = 0; }

	//CONVERTING NUMBERS
	template<typename T>
	T convert_to_send(T number);
	template<typename T>
	T convert_to_receive(T number);

	//RAW INSERTS
	void insert_raw(const std::string &str);

	//RAW RECEIVES
	void receive_raw(std::string &str, size_t str_size);

	//INSERTS WITH TASK CONVENTION
	template<typename T>
	void insert(T number);
	void insert(ClientMessageToServerType message);
	void insert(GameState state);
	void insert(Direction direction);
	void insert(const std::string &str);
	void insert(Position &position);
	void insert(Player &player);
	void insert(Bomb &bomb);
	void insert(Event &event);
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
	void receive(Event &event);
	template<typename T>
	void receive_list(std::list<T> &list);
	template<typename T, typename U>
	void receive_map(std::map<T, U> &map);


	// SENDING TO SERVER
	void send_join(std::string &string);
	void send_place_bomb();
	void send_place_block();
	void send_move(Direction direction);

	// RECEIVING FROM SERVER
	void receive_hello(ReceivedServerMessageUnion &message);
	void receive_accepted_player(ReceivedServerMessageUnion &message);
	void receive_game_started(ReceivedServerMessageUnion &message);
	void receive_turn(ReceivedServerMessageUnion &message);
	void receive_game_ended(ReceivedServerMessageUnion &message);

	// SENDING TO DISPLAY
	void send_lobby(DrawMessageUnion &message);
	void send_game(DrawMessageUnion &message);

public:
	size_t send_to_server(ClientMessageToServerType clientMessage,
	                      std::string name = std::string(), Direction direction = {});

	void receive_from_server(std::optional<ServerMessageToClient> &serverMessage);

	size_t send_to_display(ClientMessageToDisplay &drawMessage);

	void receive_from_display(std::optional<DisplayMessageToClient> &message);

	size_t get_size() const { return send_index; }

	char get_message_id() { return buffer[0]; }

	char *get() { return buffer; }

private:
	char buffer[BUFFER_SIZE]{};
	size_t send_index{0};
	size_t read_index{0};
};


#endif//ZADANIE02_BUFFER_H
