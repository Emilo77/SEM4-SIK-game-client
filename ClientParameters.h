#ifndef ZADANIE02_CLIENTPARAMETERS_H
#define ZADANIE02_CLIENTPARAMETERS_H

#include <boost/program_options.hpp>
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
	std::string server_address;
	bool help_enabled{false};
	std::string player_name;
	uint16_t port{0};

	ClientParameters(int argc, char *argv[]) : argc(argc), argv(argv) {
		check_parameters();
	}
};


#endif //ZADANIE02_CLIENTPARAMETERS_H
