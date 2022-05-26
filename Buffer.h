#ifndef ZADANIE02_BUFFER_H
#define ZADANIE02_BUFFER_H

#define BUFFER_SIZE 65535

#include "ClientParameters.h"
#include "Utils.h"
#include "Messages.h"
#include <cstring>
#include <optional>

using std::string;

class Buffer {
private:
	/* Konwertowanie liczby przed wysłaniem wiadomości */
	static uint16_t convert_to_send(uint16_t number);

	static uint32_t convert_to_send(uint32_t number);

	/* Konwertowanie liczby po odebraniu wiadomości */
	static uint16_t convert_to_receive(uint16_t number);

	static uint32_t convert_to_receive(uint32_t number);

	/* Wstawianie napisu (bez jego długości) */
	void insert_raw(const std::string &str);

	/* Odbieranie napisu */
	void receive_raw(std::string &str, size_t str_size);

	/* Wstawianie typów liczbowych */
	void insert(uint8_t number);

	void insert(uint16_t number);

	void insert(uint32_t number);

	/* Wstawianie obiektów do bufora zgodnie z konwencją zadania */
	void insert(const std::string &str);

	void insert(Position &position);

	void insert(Player &player);

	void insert(Bomb &bomb);

	/* Wstawianie list */
	void insert_list_positions(std::list<Position> &positions);

	void insert_list_bombs(std::list<Bomb> &bombs);

	/* Wstawianie map */
	void insert_map_players(std::map<player_id_t, Player> &players);

	void insert_map_scores(std::map<player_id_t, score_t> &scores);

	void insert_map_positions(std::map<player_id_t, Position> &positions);

	/* Odbieranie typów liczbowych */
	void receive(uint8_t &number);

	void receive(uint16_t &number);

	void receive(uint32_t &number);

	/* Wstawianie obiektów zgodnie z konwencją zadania */
	void receive(std::string &str);

	void receive(Position &position);

	void receive(Player &player);

	/* Odbieranie eventu */
	struct BombPlaced receive_bomb_placed();

	struct BombExploded receive_bomb_exploded();

	struct PlayerMoved receive_player_moved();

	struct BlockPlaced receive_block_placed();

	Event receive_event();

	/* Odbieranie list */
	void receive_list_events(std::vector<Event> &vector);

	void receive_list_player_ids(std::vector<player_id_t> &ids);

	void receive_list_positions(std::vector<Position> &positions);

	/* Odbieranie map */
	void receive_map_players(std::map<player_id_t, Player> &players);

	void receive_map_scores(std::map<player_id_t, score_t> &scores);

	/* Wstawianie wiadomości wysyłanych do serwera */
	void insert_join(std::string &name);

	void insert_place_bomb();

	void insert_place_block();

	void insert_move(Direction direction);

	/* Odbieranie wiadomości wysyłanych od serwera */
	struct Hello receive_hello();

	struct AcceptedPlayer receive_accepted_player();

	struct GameStarted receive_game_started();

	struct Turn receive_turn();

	struct GameEnded receive_game_ended();

	/* Wstawianie wiadomości wysyłanych do gui */
	void send_lobby(Lobby &message);

	void send_game(GamePlay &message);



public:
	/* Przywrócenie indeksu przed odbieraniem nowej wiadomości */
	void reset_read_index() { read_index = 0; }

	/* Przywrócenie indeksu przed wysyłaniem nowej wiadomości */
	void reset_send_index() { send_index = 0; }

	/* Przywrócenie indeksu przed wysyłaniem nowej wiadomości */
	void set_shift(size_t value) { send_index = value; }

	/* Wstawianie wiadomości wysyłanej do serwera */
	size_t insert_msg_to_server(ClientMessageToServer &message);

	/* Odbieranie wiadomości wysyłanej od serwera */
	std::optional<ServerMessageToClient>
	receive_msg_from_server(size_t received_size);

	/* Wstawianie wiadomości wysyłanej do gui */
	size_t insert_msg_to_display(ClientMessageToDisplay &drawMessage);

	/* Odbieranie wiadomości wysyłanej od gui */
	std::optional<DisplayMessageToClient>
	receive_msg_from_gui(size_t expected_size);

	/* Zwrócenie wielkości zapisanej wiadomości do bufora */
	[[nodiscard]] size_t get_send_size() const { return send_index; }

	/* Zwrócenie wielkości odebranej wiadomości z bufora */
	[[nodiscard]] size_t get_read_size() const { return read_index; }

	/* Zwrócenie wielkości odebranej wiadomości z bufora */
	[[nodiscard]] size_t get_shift() const { return shift_index; }

	/* Wskaźnik na bufor */
	char *get() { return buffer; }

	void print(size_t size) {
		std::cerr << "size: " << size << std::endl;
		for (size_t i = 0; i < size; i++) {
			std::cerr << (int) buffer[i] << " | ";
		}
		std::cerr << std::endl;
	}

private:
	char buffer[BUFFER_SIZE]{};
	size_t send_index{0};
	size_t read_index{0};
	size_t shift_index{0};
};


#endif//ZADANIE02_BUFFER_H
