#ifndef ZADANIE02_BUFFER_H
#define ZADANIE02_BUFFER_H

#define BUFFER_SIZE 65507

#include "ClientParameters.h"
#include "Utils.h"

class Buffer {
private:
	// we begin from 1, because we already know the value of buffer[0]- message_id
	void reset_read_index() { read_index = 1; }
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
	void insert(ClientMessageToServer message);
	void insert(Direction direction);
	void insert(const std::string &str);
	void insert(Position &position);
	void insert(Player &player);
	template<typename T>
	void insert_list(std::list<T> &list);
	template<typename T, typename U>
	void insert_map(std::map<T, U> &map);



	template<typename T>
	void receive(T &number);
	void receive(std::string &str);
	void receive(Player &player);
	void receive(Event &event);
	template<typename T>
	void receive_list(std::list<T> &list);
	template<typename T, typename U>
	void receive_map(std::map<T, U> &map);


	void send_join(std::string &string);
	void send_place_bomb();
	void send_place_block();
	void send_move(Direction direction);

	void receive_hello(ServerMessageUnion &message);
	void receive_accepted_player(ServerMessageUnion &message);
	void receive_game_started(ServerMessageUnion &message);
	void receive_turn(ServerMessageUnion &message);
	void receive_game_ended(ServerMessageUnion &message);



public:
	size_t send_to_server(ClientMessageToServer clientMessage,
	                      std::string name = std::string(), Direction direction = {});

	void receive_from_server(ServerMessage &serverMessage);

	size_t get_size() const { return send_index; }

	char get_message_id() { return buffer[0]; }

	char *get() { return buffer; }

private:
	char buffer[BUFFER_SIZE]{};
	size_t send_index{0};
	size_t read_index{1};
};


#endif//ZADANIE02_BUFFER_H
