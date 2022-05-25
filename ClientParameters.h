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

	/* Sprawdzenie ogólne parametrów */
	void check_parameters();

	void print_parameters() const { //usunąć później
		std::cout << "ClientParameters: " << std::endl;

		std::cout << "          " << "gui_address: " << gui_address << std::endl;
		std::cout << "          " << "server_address: " << server_address << std::endl;

		std::cout << "          " << "gui_host: " << gui_host << std::endl;
		std::cout << "          " << "server_host: " << server_host << std::endl;

		std::cout << "          " << "gui_port: " << gui_port << std::endl;
		std::cout << "          " <<"server_port: " << server_port << std::endl;

		std::cout << "          " <<"port: " << port << std::endl;
		std::cout << "          " <<"player_name: " << player_name << std::endl;
	}

public:
	std::string gui_address;
	std::string server_address;
	std::string gui_host;
	std::string server_host;
	uint16_t gui_port{0};
	uint16_t server_port{0};
	std::string player_name;
	uint16_t port{0};

	ClientParameters(int argc, char *argv[]) : argc(argc), argv(argv) {
		check_parameters();
		print_parameters();
	}
};


#endif //ZADANIE02_CLIENTPARAMETERS_H
