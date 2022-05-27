#include "ClientParameters.h"


namespace po = boost::program_options;

/* Pomocnicza funkcja do uproszczenia działania biblioteki Boost */
template<class T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
	copy(v.begin(), v.end(), ostream_iterator<T>(os, " "));
	return os;
}

/* Zakończenie programu w przypadku podania błędnych parametrów */
void exit_program(int status) {
	if (status) {
		std::cerr << "Consider using -h [--help] option" << std::endl;
	}
	exit(status);
}

/* Podział adresu ip na host i port */
void split_ip(std::string &address, std::string &host, std::string &port) {
	size_t pos = address.find_last_of(':');
	if (pos == std::string::npos) {
		return;
	} else {
		host = address.substr(0, pos);
		port = address.substr(pos + 1);
	}
}

/* Sprawdzenie, czy port mieści się w dozwolonym przedziale */
static inline void check_port(int possible_port) {
	if (possible_port < 0 || possible_port > 65535) {
		throw po::validation_error(po::validation_error::invalid_option_value,
		                           "port");
	}
}

/* Sprawdzenie, czy port da się poprawnie konwertować z napisu do liczby */
static inline void check_port_str(std::string &port_str) {
	try {
		/* Sprawdzamy, czy jest możliwa konwersja do postaci uint16_t. */
		boost::numeric_cast<uint16_t>(boost::lexical_cast<int>(port_str));
	} catch (boost::bad_lexical_cast &) {
		/* W przypadku błędu łapiemy wyjątek */
		std::cerr << "Invalid port format in ip" << std::endl;
		throw po::validation_error(po::validation_error::invalid_option_value,
		                           "address port");
	} catch (boost::bad_numeric_cast &) {
		/* W przypadku błędu łapiemy wyjątek */
		std::cerr << "Port out of range in ip" << std::endl;
		throw po::validation_error(po::validation_error::invalid_option_value,
		                           "address port");
	}
}

/* Częściowe sprawdzenie poprawności adresu ip */
static inline void check_address(std::string ip) {
	std::string host, port;
	split_ip(ip, host, port);
	check_port_str(port);
}

/* Sprawdzamy, czy adres GUI i adres serwera nie są takie same. */
void ClientParameters::compare_address() const {
	if (gui_address == server_address) {
		throw po::validation_error(po::validation_error::invalid_option_value,
		                           "display and server addresses are the same");
	}
}


void ClientParameters::check_parameters() {
	/* Na początku zapisujemy port do inta, aby móc sprawdzić jego poprawność. */
	int possible_port = -1;
	const po::positional_options_description p;
	po::options_description desc(1024, 512);
	try {
		desc.add_options()
				("help,h", "produce help message")
				("gui-address,d",
				 po::value<std::string>(&gui_address)->value_name
								 ("<(hostname):(port) or (IPv4):(port) or (IPv6):(port)>")
						 ->required()->notifier(&check_address),
				 "set the gui address")
				("player-name,n",
				 po::value<std::string>(&player_name)->value_name("String")
						 ->required(),
				 "set the player name")
				("port,p", po::value<int>(&possible_port)->value_name("u16")
						 ->required()->notifier(&check_port),
				 "set the port number")
				("server-address,s",
				 po::value<std::string>(&server_address)->value_name
								 ("<(hostname):(port) or (IPv4):(port) or (IPv6):(port)>")
						 ->required()->notifier(&check_address),
				 "set the server address");

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).
				options(desc).
				positional(p).
				run(), vm);

		/* Jeżeli wystąpi opcja help, ignorujemy inne parametry
		 * i kończymy działanie programu */
		if (vm.count("help")) {
			std::cout << "Program Usage: ./" << argv[0] << "\n" << desc << "\n";
			exit_program(0);
		}

		po::notify(vm);

		compare_address();

		/* Dzielimy adresy na hosty i porty, zapisujemy je w parametrach. */
		split_ip(server_address, server_host, server_port);
		split_ip(gui_address, gui_host, gui_port);

	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << "\n";
		exit_program(1);
	}
	catch (...) {
		std::cerr << "Unknown error!" << "\n";
		exit_program(1);
	}
	port = (uint16_t) possible_port;
}
