#ifndef ZADANIE02_CLIENTPARAMETERS_H
#define ZADANIE02_CLIENTPARAMETERS_H

#include <boost/program_options.hpp>
#include <boost/asio/ip/address.hpp>
#include <string>
#include <iostream>
#include <fstream>

class ClientParameters {
private:
	int argc{0};
	char **argv;

	/* Sprawdzenie, czy serwer i gui posiadają ten sam adres ip */
	void compare_address() const;

	/* Główna funkcja sprawdzająca poprawność parametrów. */
	void check_parameters();

public:
	std::string gui_address;
	std::string server_address;
	/* Hosty */
	std::string gui_host;
	std::string server_host;
	/* Przechowujemy poprawne porty w napisach, ponieważ do wyznaczania
	 * endpointów w bibliotece Boost podaje się porty w takiej postaci. */
	std::string gui_port;
	std::string server_port;
	std::string player_name;
	/* Port do nasłuchiwania komunikatów od GUI. */
	uint16_t port{0};

	ClientParameters(int argc, char *argv[]) : argc(argc), argv(argv) {
		check_parameters();
	}
};


#endif //ZADANIE02_CLIENTPARAMETERS_H
