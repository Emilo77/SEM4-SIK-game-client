#ifndef ZADANIE02_BUFFER_H
#define ZADANIE02_BUFFER_H

#define BUFFER_SIZE 65507

#include "Utils.h"
#include "ServerParameters.h"
#include "Game.h"

class Buffer {
private:
	template<typename T>
	T convert_to_send(T number);

	template<typename T>
	T convert_to_receive(T number);

	template<typename T>
	void insert(T number);

	void insert(const std::string &str);

	void insert(Position &position);

	template<typename T>
	//todo ograniczenie na T, że musi być primitive

	void insert(std::list<T> &list);

	template<typename T>
	void receive_number(T &number);

	void receive_string(std::string &str, size_t str_size);

// we begin from 1, because we already know the value of buffer[0]- message_id
	void reset_read_index() { read_index = 1; }

	void reset_send_index() { send_index = 0; }


public:
	void receive_hello();

	void receive_accpted_player();

	void receive_game_started();

	void receive_turn();

	void receive_game_ended();


	size_t get_size() const { return send_index; }

	char get_message_id() { return buffer[0]; }

	char *get() { return buffer; }

private:
	char buffer[BUFFER_SIZE]{};
	size_t send_index{0};
	size_t read_index{1};
};


#endif //ZADANIE02_BUFFER_H
