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
	enum wrong_parameters {
		//todo
	};

	void check_parameters();

	void print_parameters() const {
		std::cout << "ClientParameters: " << std::endl;
		std::cout << "          " <<"display_address: " << display_address << std::endl;
		std::cout << "          " <<"server_address: " << server_address << std::endl;
		std::cout << "          " <<"port: " << port << std::endl;
		std::cout << "          " <<"player_name: " << player_name << std::endl;
	}

public:
	std::string display_address; //może potem zamienić na inny typ
	std::string server_address;
	uint16_t display_port;
	uint16_t server_port;
	std::string player_name;
	uint16_t port{0};

	ClientParameters(int argc, char *argv[]) : argc(argc), argv(argv) {
		check_parameters();
		print_parameters();
	}
};


#endif //ZADANIE02_CLIENTPARAMETERS_H
