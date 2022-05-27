#include "GameInfo.h"

void Board::reset(uint16_t size_x, uint16_t size_y) {
	/* Zmieniamy rozmiar planszy. */
	fields.resize(size_x);
	for (auto &row: fields) {
		row.resize(size_y);
	}

	/* Resetujemy wszystkie pola. */
	for (auto &column: fields) {
		for (auto &field: column) {
			field.make_air();
			field.reset_exploded();
		}
	}
}

std::list<Position> Board::return_blocks() {
	std::list<Position> blocks;
	/* Iterujemy się po całej planszy. */
	for (size_t column = 0; column < fields.size(); column++) {
		for (size_t row = 0; row < fields[column].size(); row++) {
			/* Jeżeli pole jest blokiem, dodajemy jego pozycję do listy. */
			if (fields[column][row].is_solid()) {
				blocks.emplace_back(column, row);
			}
		}
	}
	return blocks;
}

std::list<Position> Board::return_explosions() {
	std::list<Position> exploded;
	/* Iterujemy się po całej planszy. */
	for (size_t column = 0; column < fields.size(); column++) {
		for (size_t row = 0; row < fields[column].size(); row++) {
			/* Jeżeli pole eksplodowało w aktualnej turze, dodajemy jego
			 * pozycję do listy, zmieniamy pole na nie-blok
			 * oraz resetujemy stan eksplozji. */
			if (fields[column][row].is_exploded()) {
				exploded.emplace_back(column, row);
				fields[column][row].make_air();
				fields[column][row].reset_exploded();
			}
		}
	}
	return exploded;
}

/* Funkcja pomocnicza do obliczania eksplozji po wybuchu bomby. */
static inline std::pair<int, int> direction_to_pair(Direction direction) {
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
	return {0, 0};
}

void GameInfo::apply_changes_from_server(ServerMessageToClient &msg) {
	/* Na podstawie typu wiadomości, aktualizujemy stan rozgrywki. */
	switch (msg.type) {
		case Hello:
			apply_Hello(std::get<struct Hello>(msg.data));
			break;
		case AcceptedPlayer:
			apply_AcceptedPlayer(std::get<struct AcceptedPlayer>(msg.data));
			break;
		case GameStarted:
			apply_GameStarted(std::get<struct GameStarted>(msg.data));
			break;
		case Turn:
			apply_Turn(std::get<struct Turn>(msg.data));
			break;
		case GameEnded:
			apply_GameEnded(std::get<struct GameEnded>(msg.data));
			break;
	}
}

void GameInfo::apply_Hello(struct Hello &message) {
	/* Resetujemy kontenery. */
	clear_containers();

	/* Aktualizujemy parametry na podstawie wiadomości. */
	server_name = message.server_name;
	players_count = message.players_count;
	size_x = message.size_x;
	size_y = message.size_y;
	game_length = message.game_length;
	explosion_radius = message.explosion_radius;
	bomb_timer = message.bomb_timer;

	/* Resetujemy planszę. */
	board.reset(message.size_x, message.size_y);
}

void GameInfo::apply_AcceptedPlayer(struct AcceptedPlayer &message) {
	players.insert_or_assign(message.player_id, message.player);
}

void GameInfo::apply_GameStarted(struct GameStarted &message) {
	/* Zmieniamy stan na Gameplay. */
	change_game_state(GameState::GameplayState);
	players = message.players;
	/* Aktualizujemy kontenery na podstawie otrzymanej listy graczy. */
	initialize_containers();
}

void GameInfo::apply_Turn(struct Turn &message) {
	/* Czyścimy eksplozje z poprzedniej tury. */
	explosions.clear();
	/* Zmniejszamy timer dla każdej bomby. */
	decrease_bomb_timers();
	turn = message.turn_number;
	/* Aktualizujemy stan gry na podstawie każdego otrzymanego wydarzenia. */
	for (auto &event: message.events) {
		apply_event(event);
	}
	/* Przywracamy graczy, aktualizujemy ich scores. */
	change_scores_and_revive_players();
	/* Aktualizujemy eksplozje, czyścimy zniszczone bloki. */
	explosions = board.return_explosions();
}

void GameInfo::apply_GameEnded(struct GameEnded &message) {
	scores = message.scores;
	/* Zmieniamy stan na Lobby. */
	change_game_state(GameState::LobbyState);
	clear_containers();
}

void GameInfo::apply_BombPlaced(struct BombPlaced &data) {
	bombs.insert_or_assign(data.bomb_id, Bomb( data.position, bomb_timer));
}

void GameInfo::apply_BombExploded(struct BombExploded &data) {
	/* Oznaczamy pola, które eksplodowały przez bombę. */
	mark_explosions(data);

	/* Oznaczamy graczy zniszczonych przez bombę. */
	for (auto robot_id: data.robots_destroyed) {
		if (players.find(robot_id) != players.end()) {
			players.at(robot_id).explode();
		}
	}

	/* Oznaczamy zniszczone pola. */
	for (auto &position: data.blocks_destroyed) {
		board.at(position).mark_exploded();
	}

	/* Usuwamy bombę z listy. */
	bombs.erase(data.bomb_id);
}

void GameInfo::apply_PlayerMoved(struct PlayerMoved &data) {
	/* Zmieniamy aktualną pozycję gracza. */
	if (players.find(data.player_id) != players.end()) {
		player_positions.at(data.player_id) = data.position;
	}
}

void GameInfo::apply_BlockPlaced(struct BlockPlaced &data) {
	board.at(data.position).make_block();
}

void GameInfo::apply_event(Event &event) {
	/* Aktualizujemy stan gry na podstawie wydarzenia. */
	switch (event.type) {
		case BombPlaced:
			apply_BombPlaced(std::get<struct BombPlaced>(event.data));
			break;
		case BombExploded:
			apply_BombExploded(std::get<struct BombExploded>(event.data));
			break;
		case PlayerMoved:
			apply_PlayerMoved(std::get<struct PlayerMoved>(event.data));
			break;
		case BlockPlaced:
			apply_BlockPlaced(std::get<struct BlockPlaced>(event.data));
			break;
	}
}

/* Funkcja sprawdzająca, czy pozycja mieści się w zakresie planszy. */
bool GameInfo::is_correct_position(Position position) const {
	return position.x < size_x && position.y < size_y;
}

void GameInfo::clear_containers() {
	players.clear();
	player_positions.clear();
	bombs.clear();
	scores.clear();
	explosions.clear();

}

void GameInfo::initialize_containers() {
	for (auto &new_player_pair: players) {
		scores.insert({new_player_pair.first, 0});
		player_positions.insert({new_player_pair.first, Position()});
	}
}

void GameInfo::decrease_bomb_timers() {
	for (auto &bombs_element: bombs) {
		bombs_element.second.decrease_timer();
	}
}

void GameInfo::change_scores_and_revive_players() {
	for (auto &player_pair: players) {
		if (player_pair.second.is_dead()) {
			scores.at(player_pair.first)++;
			player_pair.second.revive();
		}
	}
}

void
GameInfo::mark_explosions_in_direction(Position bomb_pos, Direction direction) {
	auto pair = direction_to_pair(direction);

	for (int i = 1; i <= explosion_radius; i++) {
		int new_x = bomb_pos.x + i * pair.first;
		int new_y = bomb_pos.y + i * pair.second;

		/* Otrzymujemy nowe pozycje w określonym kierunku od bomby. */
		Position new_pos(static_cast<uint16_t>(new_x),
		                 static_cast<uint16_t>(new_y));


		/* Sprawdzamy, czy eksplozja mieści się w zakresie planszy. */
		if (is_correct_position(new_pos)) {
			/* Oznaczamy pole */
			board.at(new_pos).mark_exploded();
			/* Jeżeli eksplodował blok, wychodzimy z pętli,
			 * ponieważ eksplozje nie docierają za blokami. */
			if (board.at(new_pos).is_solid())
				break;
		}
	}
}

void GameInfo::mark_explosions(struct BombExploded &data) {

	Position bomb_pos = bombs.at(data.bomb_id).position;

	/* Oznaczamy pole na pozycji bomby. */
	board.at(bomb_pos).mark_exploded();

	/* Jeżeli bomba eksplodowała na bloku, kończymy działanie,
	 * bomba rozsadza tylko pojedynczy blok, na którym została położona. */
	if (board.at(bomb_pos).is_solid()) {
		return;
	}

	/* W przeciwnym wypadku obliczamy eksplozje w czterech kierunkach od bomby.*/
	mark_explosions_in_direction(bomb_pos, Up);
	mark_explosions_in_direction(bomb_pos, Right);
	mark_explosions_in_direction(bomb_pos, Down);
	mark_explosions_in_direction(bomb_pos, Left);
}

bool GameInfo::is_gameplay() {
	return game_state == GameplayState;
}

void GameInfo::change_game_state(GameState state) {
	game_state = state;
}

Lobby GameInfo::create_lobby_msg() {
	/* Tworzymy wiadomość Lobby. */
	return {server_name, players_count, size_x, size_y, game_length,
	        explosion_radius, bomb_timer, players};
}

struct GamePlay GameInfo::create_gameplay_msg() {
	std::list<Position> blocks = board.return_blocks();
	std::list<Bomb> bombs_list;
	for (auto &bomb: bombs) {
		bombs_list.push_back(bomb.second);
	}
	/* Zwracamy wiadomość GamePlay. */
	return {server_name, size_x, size_y, game_length, turn, players,
	        player_positions, blocks, bombs_list, explosions, scores};
}
