#ifndef ZADANIE02_CLIENT_GAMEINFO_H
#define ZADANIE02_CLIENT_GAMEINFO_H

#include <string>
#include <map>
#include "Messages.h"
#include "Utils.h"
#include "ClientParameters.h"

/* Pomocnicza klasa przechowująca informacje o danym polu na planszy. */
class Field {
	bool solid{false};
	bool exploded{false};

public:
	/* Zaznaczenie, że pole jest blokiem. */
	void make_block() { solid = true; }

	/* Zaznaczenie, że pole jest powietrzem (nie jest blokiem). */
	void make_air() { solid = false; }

	/* Zaznaczenie, że pole eksplodowało w ostatniej turze. */
	void mark_exploded() { exploded = true; }

	/* Zresetowanie eksplozji. */
	void reset_exploded() { exploded = false; }

	/* Sprawdzenie, czy pole jest blokiem. */
	[[nodiscard]] bool is_solid() const { return solid; }

	/* Sprawdzenie, czy pole eksplodowało w aktualnej turze. */
	[[nodiscard]] bool is_exploded() const { return exploded; }
};

/* Pomocnicza klasa planszy, dzięki niej jesteśmy w stanie szybko
 * sprawdzić informacje dotyczące danego pola. */
class Board {
	std::vector<std::vector<Field>> fields;

public:

	Field &at(Position position) {
		return fields[position.x][position.y];
	}

	/* Ustawiamy planszę na odpowiedni rozmiar i czyścimy ją. */
	void reset(uint16_t size_x, uint16_t size_y);

	/* Zwrócenie listy pozycji wszystkich pól będących polami na planszy. */
	std::list<Position> return_blocks();

	/* Zwrócenie listy pozycji wszystkich pól, które eksplodowały. */
	std::list<Position> return_explosions();
};

class GameInfo {
private:
	enum GameState game_state{LobbyState};

	/* Potrzebne informacje, kiedy gra znajduje się w stanie Lobby. */
	std::string server_name{};
	uint8_t players_count{0};
	uint16_t size_x{0};
	uint16_t size_y{0};
	uint16_t game_length{0};
	uint16_t explosion_radius{0};
	uint16_t bomb_timer{0};
	std::map<player_id_t, Player> players;

	/* Potrzebne informacje, kiedy gra znajduje się w stanie GamePlay. */
	Board board;
	uint16_t turn{0};
	std::map<player_id_t, Position> player_positions;
	std::map<bomb_id_t, Bomb> bombs;
	std::map<player_id_t, score_t> scores;
	std::list<Position> explosions;

public:
	/* Aktualizacja stanu gry, na podstawie wiadomości od serwera. */
	void apply_changes_from_server(ServerMessageToClient &msg);

	/* Sprawdzenie, w jakim stanie jest gra. */
	bool is_gameplay();

	/* Zmiana stanu gry. */
	void change_game_state(GameState state);

	/* Stworzenie wiadomości wysyłanej do Gui. */
	Lobby create_lobby_msg();

	/* Stworzenie wiadomości wysyłanej do Gui. */
	GamePlay create_gameplay_msg();

private:

	/* Sprawdzenie, czy pozycja mieści się w zakresie planszy. */
	[[nodiscard]] bool is_correct_position(Position position) const;

	/* Czyszczenie kontenerów z informacjami. */
	void clear_containers();

	/* Dopasowanie wielkości kontenerów na podstawie graczy. */
	void initialize_containers();

	/* Zmniejszenie timera dla wszystkich bomb na planszy. */
	void decrease_bomb_timers();

	/* Zapisanie eksplozji pojedynczej bomby w danym kierunku. */
	void mark_explosions_in_direction(Position bomb_pos, Direction direction);

	/* Zapisanie eksplozji pojedynczej bomby. */
	void mark_explosions(struct BombExploded &data);

	/* Aktualizacja wyników graczy. */
	void change_scores_and_revive_players();

	/* Aktualizacja stanu gry na podstawie zdarzenia */
	void apply_event(Event &event);

	/* Aktualizacja stanu gry na podstawie wiadomości Hello. */
	void apply_Hello(struct Hello &message);

	/* Aktualizacja stanu gry na podstawie wiadomości AcceptedPlayer. */
	void apply_AcceptedPlayer(struct AcceptedPlayer &message);

	/* Aktualizacja stanu gry na podstawie wiadomości AcceptedPlayer. */
	void apply_GameStarted(struct GameStarted &message);

	/* Aktualizacja stanu gry na podstawie wiadomości Turn. */
	void apply_Turn(struct Turn &message);

	/* Aktualizacja stanu gry na podstawie wiadomości GameEnded. */
	void apply_GameEnded(struct GameEnded &message);

	/* Aktualizacja stanu gry na podstawie zdarzenia BombPlaced. */
	void apply_BombPlaced(struct BombPlaced &data);

	/* Aktualizacja stanu gry na podstawie zdarzenia BombExploded. */
	void apply_BombExploded(struct BombExploded &data);

	/* Aktualizacja stanu gry na podstawie zdarzenia PlayerMoved. */
	void apply_PlayerMoved(struct PlayerMoved &data);

	/* Aktualizacja stanu gry na podstawie zdarzenia BlockPlaced. */
	void apply_BlockPlaced(struct BlockPlaced &data);
};

#endif//ZADANIE02_CLIENT_GAMEINFO_H
