
//
// Created by emilo77 on 07.05.22.
//

#ifndef ZADANIE02_CLIENTPARAMETERS_H
#define ZADANIE02_CLIENTPARAMETERS_H

#include <string>


class ClientParameters {
private:
	int argc{0};
	char **argv;
	enum wrong_parameters {
		//todo
	};

	void exit_program(enum wrong_parameters);

	void check_parameters();

public:
	std::string display_address; //może potem zamienić na inny typ
	bool help_enabled{false};
	std::string player_name;
	uint16_t port{0};
	std::string server_address;

	ClientParameters(int argc, char *argv[]) : argc(argc), argv(argv) {
		check_parameters();
	}
};


#endif //ZADANIE02_CLIENTPARAMETERS_H
